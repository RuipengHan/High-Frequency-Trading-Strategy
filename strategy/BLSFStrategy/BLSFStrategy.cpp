#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "BLSFStrategy.h"

#include "FillInfo.h"
#include "AllEventMsg.h"
#include "ExecutionTypes.h"
#include <Utilities/Cast.h>
#include <Utilities/utils.h>

#include <math.h>
#include <iostream>
#include <cassert>
#include <ctime>

using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

using namespace std;

BLSFStrategy::BLSFStrategy(StrategyID strategyID, const std::string& strategyName, const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    currentState(BUY),
    currentDate(0) {
}

BLSFStrategy::~BLSFStrategy(){}

void BLSFStrategy::OnResetStrategyState()
{
}

void BLSFStrategy::DefineStrategyParams()
{
    CreateStrategyParamArgs arg1("aggressiveness", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_DOUBLE);
    params().CreateParam(arg1);
    
    CreateStrategyParamArgs arg2("debug", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_BOOL);
    params().CreateParam(arg2);
}

void BLSFStrategy::DefineStrategyCommands()
{
    StrategyCommand command1(1, "Reprice Existing Orders");
    commands().AddCommand(command1);

    StrategyCommand command2(2, "Cancel All Orders");
    commands().AddCommand(command2);
}

void BLSFStrategy::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate)
{    
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 10);
    }
}

void BLSFStrategy::OnTrade(const TradeDataEventMsg& msg)
{

    date msg_date = msg.adapter_time().date();
    // Sell at the beginning of the day
    if (currentState == SELL && msg_date != currentDate) {
        currentDate = msg_date;
        std::cout << "OnTrade(): (" << msg.adapter_time() << "): " << msg.instrument().symbol() << ": " << msg.trade().size() << " @ $" << msg.trade().price() << std::endl;
        this->SendSimpleOrder(&msg.instrument(), -1); //sell one share every time there is a trade
        currentState = BUY;
    }
    // Buy at the end of the day
    else {
        if(currentDate == date(0)) {
            currentDate = msg_date;
        }
        tm date_tm = to_tm(msg.adapter_time());
        if(currentState == BUY && date_tm.tm_hour == 19 && date_tm.tm_min >= 58) {
            std::cout << "OnTrade(): (" << msg.adapter_time() << "): " << msg.instrument().symbol() << ": " << msg.trade().size() << " @ $" << msg.trade().price() << std::endl;
            this->SendSimpleOrder(&msg.instrument(), 1);
            cout << date_tm.tm_hour << "\t" << date_tm.tm_min << endl;
            currentState = SELL;
        }
    }	
}

void BLSFStrategy::OnBar(const BarEventMsg& msg)
{
    date msg_date = msg.bar_time().date();
    if (currentState == SELL && msg_date != currentDate) {
        tm date_tm = to_tm(msg.bar_time());
        // ensure the market begin time 
        if(currentState == BUY && date_tm.tm_hour < 13) {
            return;
        }
        if (date_tm.tm_hour == 13 && date_tm.tm_min < 30) {
            return;
        } 
        currentDate = msg_date;
        std::cout << "Sending BAR order: (" << msg.bar_time() << "): " << msg.instrument().symbol() << std::endl;
        this->SendOrder(&msg.instrument(), -1); //sell one share every time there is a trade
        currentState = BUY;
    }
    // Buy at the end of the day
    else {
        if(currentDate == date(0)) {
            currentDate = msg_date;
        }
        tm date_tm = to_tm(msg.bar_time());
        if(currentState == BUY && date_tm.tm_hour == 19 && date_tm.tm_min >= 58) {
            std::cout << "Sending BAR order: (" << msg.bar_time() << "): " << msg.instrument().symbol() << std::endl;
            this->SendOrder(&msg.instrument(), 1);
            currentState = SELL;
        }
    }	
}

void BLSFStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg)
{    
	std::cout << "OnOrderUpdate(): " << msg.update_time() << " " << msg.name() << std::endl;
    if(msg.completes_order())
    {
		std::cout << "OnOrderUpdate(): order is complete; " << std::endl;
    }
}

void BLSFStrategy::AdjustPortfolio(const Instrument* instrument, int desired_position)
{
}

void BLSFStrategy::SendSimpleOrder(const Instrument* instrument, int trade_size)
{
    double last_trade_price = instrument->last_trade().price();
    double price = trade_size > 0 ? last_trade_price : last_trade_price;

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
        std::cout << "SendOrder(): Sending new order successful!" << std::endl;
    }
    else {
        std::cout << "SendOrder(): Error sending new order!!!" << tra << std::endl;
    }

}


void BLSFStrategy::SendOrder(const Instrument* instrument, int trade_size)
{
    if(instrument->top_quote().ask()<.01 || instrument->top_quote().bid()<.01 || !instrument->top_quote().ask_side().IsValid() || !instrument->top_quote().ask_side().IsValid()) {
        std::stringstream ss;
        ss << "Sending buy order for " << instrument->symbol() << " at price " << instrument->top_quote().ask() << " and quantity " << trade_size <<" with missing quote data";   
        logger().LogToClient(LOGLEVEL_DEBUG, ss.str());
        std::cout << "SendOrder(): " << ss.str() << std::endl;
        return;
    }

    double price = trade_size > 0 ? instrument->top_quote().bid() : instrument->top_quote().ask();

    OrderParams params(*instrument, 
        abs(trade_size),
        price, 
        (instrument->type() == INSTRUMENT_TYPE_EQUITY) ? MARKET_CENTER_ID_NASDAQ : ((instrument->type() == INSTRUMENT_TYPE_OPTION) ? MARKET_CENTER_ID_CBOE_OPTIONS : MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size>0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);
        
    trade_actions()->SendNewOrder(params);
}

void BLSFStrategy::RepriceAll()
{
    for (IOrderTracker::WorkingOrdersConstIter ordit = orders().working_orders_begin(); ordit != orders().working_orders_end(); ++ordit) {
        Reprice(*ordit);
    }
}

void BLSFStrategy::Reprice(Order* order)
{
    OrderParams params = order->params();
    params.price = (order->order_side() == ORDER_SIDE_BUY) ? order->instrument()->top_quote().bid() : order->instrument()->top_quote().ask();
    trade_actions()->SendCancelReplaceOrder(order->order_id(), params);
}