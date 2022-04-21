/*================================================================================                               
*     Source: ../RCM/StrategyStudio/examples/strategies/SimpleMomentumStrategy/SimpleMomentumStrategy.cpp                                                        
*     Last Update: 2013/6/1 13:55:14                                                                            
*     Contents:                                     
*     Distribution:          
*                                                                                                                
*                                                                                                                
*     Copyright (c) RCM-X, 2011 - 2013.                                                  
*     All rights reserved.                                                                                       
*                                                                                                                
*     This software is part of Licensed material, which is the property of RCM-X ("Company"), 
*     and constitutes Confidential Information of the Company.                                                  
*     Unauthorized use, modification, duplication or distribution is strictly prohibited by Federal law.         
*     No title to or ownership of this software is hereby transferred.                                          
*                                                                                                                
*     The software is provided "as is", and in no event shall the Company or any of its affiliates or successors be liable for any 
*     damages, including any lost profits or other incidental or consequential damages relating to the use of this software.       
*     The Company makes no representations or warranties, express or implied, with regards to this software.                        
/*================================================================================*/   

#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "SimpleTradeStrategy.h"

#include "FillInfo.h"
#include "AllEventMsg.h"
#include "ExecutionTypes.h"
#include <Utilities/Cast.h>
#include <Utilities/utils.h>

#include <math.h>
#include <iostream>
#include <cassert>

using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

using namespace std;

SimpleTrade::SimpleTrade(StrategyID strategyID, const std::string& strategyName, const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    m_momentum_map(),
    m_instrument_order_id_map(),
    m_momentum(0),
    m_aggressiveness(0),
    m_position_size(100),
    m_debug_on(true),
    m_short_window_size(10),
    m_long_window_size(30)
{
    //this->set_enabled_pre_open_data_flag(true);
    //this->set_enabled_pre_open_trade_flag(true);
    //this->set_enabled_post_close_data_flag(true);
    //this->set_enabled_post_close_trade_flag(true);
}

SimpleTrade::~SimpleTrade()
{
}

void SimpleTrade::OnResetStrategyState()
{
    m_momentum_map.clear();
    m_instrument_order_id_map.clear();
    m_momentum = 0;
}

void SimpleTrade::DefineStrategyParams()
{
    CreateStrategyParamArgs arg1("aggressiveness", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_DOUBLE, m_aggressiveness);
    params().CreateParam(arg1);

    CreateStrategyParamArgs arg2("position_size", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_INT, m_position_size);
    params().CreateParam(arg2);

    CreateStrategyParamArgs arg3("short_window_size", STRATEGY_PARAM_TYPE_STARTUP, VALUE_TYPE_INT, m_short_window_size);
    params().CreateParam(arg3);
    
    CreateStrategyParamArgs arg4("long_window_size", STRATEGY_PARAM_TYPE_STARTUP, VALUE_TYPE_INT, m_long_window_size);
    params().CreateParam(arg4);
    
    CreateStrategyParamArgs arg5("debug", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_BOOL, m_debug_on);
    params().CreateParam(arg5);
}

void SimpleTrade::DefineStrategyCommands()
{
    StrategyCommand command1(1, "Reprice Existing Orders");
    commands().AddCommand(command1);

    StrategyCommand command2(2, "Cancel All Orders");
    commands().AddCommand(command2);
}

void SimpleTrade::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate)
{    
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 10);
    }
}
void SimpleTrade::OnTrade(const TradeDataEventMsg& msg)
{
	std::cout << "OnTrade(): (" << msg.adapter_time() << "): " << msg.instrument().symbol() << ": " << msg.trade().size() << " @ $" << msg.trade().price() << std::endl;
	for (int i=0; i<1; i++)
	this->SendSimpleOrder(&msg.instrument(), 1); //buy one share every time there is a trade

}
void SimpleTrade::OnBar(const BarEventMsg& msg)
{
    if (m_debug_on) {
        ostringstream str;
        str << "FINDME" << msg.instrument().symbol() << ": " << msg.bar();
        logger().LogToClient(LOGLEVEL_DEBUG, str.str().c_str());
        //std::cout << str.str().c_str() << std::endl;
    }

    if(msg.bar().close() < .01) return;


    //check if we're already tracking the momentum object for this instrument, if not create a new one
    MomentumMapIterator iter = m_momentum_map.find(&msg.instrument());
    if (iter != m_momentum_map.end()) {
        m_momentum = &iter->second;
    } else {
        m_momentum = &m_momentum_map.insert(make_pair(&msg.instrument(),Momentum(m_short_window_size,m_long_window_size))).first->second;
    }

    DesiredPositionSide side = m_momentum->Update(msg.bar().close());

    if(m_momentum->FullyInitialized()) {
        AdjustPortfolio(&msg.instrument(), m_position_size * side);
    }
}

void SimpleTrade::OnOrderUpdate(const OrderUpdateEventMsg& msg)
{    
	std::cout << "OnOrderUpdate(): " << msg.update_time() << msg.name() << std::endl;
    if(msg.completes_order())
    {
		m_instrument_order_id_map[msg.order().instrument()] = 0;
		std::cout << "OnOrderUpdate(): order is complete; " << std::endl;
    }
}

void SimpleTrade::AdjustPortfolio(const Instrument* instrument, int desired_position)
{
    int trade_size = desired_position - portfolio().position(instrument);

    if (trade_size != 0) {
        OrderID order_id = m_instrument_order_id_map[instrument];
        //if we're not working an order for the instrument already, place a new order
        if (order_id == 0) {
            SendOrder(instrument, trade_size);
        } else {  
		    //otherwise find the order and cancel it if we're now trying to trade in the other direction
            const Order* order = orders().find_working(order_id);
            if(order && ((IsBuySide(order->order_side()) && trade_size < 0) || 
			            ((IsSellSide(order->order_side()) && trade_size > 0)))) {
                trade_actions()->SendCancelOrder(order_id);
                //we're avoiding sending out a new order for the other side immediately to simplify the logic to the case where we're only tracking one order per instrument at any given time
            }
        }
    }
}

void SimpleTrade::SendSimpleOrder(const Instrument* instrument, int trade_size)
{

	//this is simple check to avoid placing orders before the order book is actually fully initialized
	//side note: if trading certain futures contracts for example, the price of an instrument actually can be zero or even negative. here we assume cash US equities so price > 0
    /*
	if(instrument->top_quote().ask()<.01 || instrument->top_quote().bid()<.01 || !instrument->top_quote().ask_side().IsValid() || !instrument->top_quote().ask_side().IsValid()) {
        std::stringstream ss;
        ss << "SendSimpleOrder(): Sending buy order for " << instrument->symbol() << " at price " << instrument->top_quote().ask() << " and quantity " << trade_size <<" with missing quote data";
        logger().LogToClient(LOGLEVEL_DEBUG, ss.str());
        std::cout << "SendSimpleOrder(): " << ss.str() << std::endl;
        return;
     }*/

    m_aggressiveness = 0.02; //send order two pennies more aggressive than BBO
    double last_trade_price = instrument->last_trade().price();
    double price = trade_size > 0 ? last_trade_price + m_aggressiveness : last_trade_price - m_aggressiveness;

    OrderParams params(*instrument,
        abs(trade_size),
        price,
        (instrument->type() == INSTRUMENT_TYPE_EQUITY) ? MARKET_CENTER_ID_IEX : ((instrument->type() == INSTRUMENT_TYPE_OPTION) ? MARKET_CENTER_ID_CBOE_OPTIONS : MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size>0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    std::cout << "SendSimpleOrder(): about to send new order for " << trade_size << " at $" << price << std::endl;
    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        m_instrument_order_id_map[instrument] = params.order_id;
        std::cout << "SendOrder(): Sending new order successful!" << std::endl;
    }
    else
    {
    	std::cout << "SendOrder(): Error sending new order!!!" << tra << std::endl;
    }

}


void SimpleTrade::SendOrder(const Instrument* instrument, int trade_size)
{
	return;
    if(instrument->top_quote().ask()<.01 || instrument->top_quote().bid()<.01 || !instrument->top_quote().ask_side().IsValid() || !instrument->top_quote().ask_side().IsValid()) {
        std::stringstream ss;
        ss << "Sending buy order for " << instrument->symbol() << " at price " << instrument->top_quote().ask() << " and quantity " << trade_size <<" with missing quote data";   
        logger().LogToClient(LOGLEVEL_DEBUG, ss.str());
        std::cout << "SendOrder(): " << ss.str() << std::endl;
        return;
     }

    double price = trade_size > 0 ? instrument->top_quote().bid() + m_aggressiveness : instrument->top_quote().ask() - m_aggressiveness;

    OrderParams params(*instrument, 
        abs(trade_size),
        price, 
        (instrument->type() == INSTRUMENT_TYPE_EQUITY) ? MARKET_CENTER_ID_NASDAQ : ((instrument->type() == INSTRUMENT_TYPE_OPTION) ? MARKET_CENTER_ID_CBOE_OPTIONS : MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size>0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    if (trade_actions()->SendNewOrder(params) == TRADE_ACTION_RESULT_SUCCESSFUL) {
        m_instrument_order_id_map[instrument] = params.order_id;
    }
}

void SimpleTrade::RepriceAll()
{
    for (IOrderTracker::WorkingOrdersConstIter ordit = orders().working_orders_begin(); ordit != orders().working_orders_end(); ++ordit) {
        Reprice(*ordit);
    }
}

void SimpleTrade::Reprice(Order* order)
{
    OrderParams params = order->params();
    params.price = (order->order_side() == ORDER_SIDE_BUY) ? order->instrument()->top_quote().bid() + m_aggressiveness : order->instrument()->top_quote().ask() - m_aggressiveness;
    trade_actions()->SendCancelReplaceOrder(order->order_id(), params);
}

void SimpleTrade::OnStrategyCommand(const StrategyCommandEventMsg& msg)
{
    switch (msg.command_id()) {
        case 1:
            RepriceAll();
            break;
        case 2:
            trade_actions()->SendCancelAll();
            break;
        default:
            logger().LogToClient(LOGLEVEL_DEBUG, "Unknown strategy command received");
            break;
    }
}

void SimpleTrade::OnParamChanged(StrategyParam& param)
{    
	/*
    if (param.param_name() == "aggressiveness") {                         
        if (!param.Get(&m_aggressiveness))
            throw StrategyStudioException("Could not get m_aggressiveness");
    } else if (param.param_name() == "position_size") {
        if (!param.Get(&m_position_size))
            throw StrategyStudioException("Could not get position size");
    } else if (param.param_name() == "short_window_size") {
        if (!param.Get(&m_short_window_size))
            throw StrategyStudioException("Could not get trade size");
    } else if (param.param_name() == "long_window_size") {
        if (!param.Get(&m_long_window_size))
            throw StrategyStudioException("Could not get long_window_size");
    } else if (param.param_name() == "debug") {
        if (!param.Get(&m_debug_on))
            throw StrategyStudioException("Could not get trade size");
    } 
    */
}
