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

/* 
    Directions:
        cp MeanReversionStrategy.so ~/Desktop/strategy_studio/backtesting/strategies_dlls/
        cd ~/Desktop/strategy_studio/backtesting
        ./StrategyServerBacktesting
        create_instance mean_reversion19 MeanReversionStrategy UIUC SIM-1001-101 dlariviere 10000000 -symbols SPY
        start_backtest 2022-04-06 2022-04-06 mean_reversion19 1

    Export:
        export_cra_file backtesting-results/BACK_mean_reversion19_2022-05-12_211711_start_04-06-2022_end_04-06-2022.cra backtesting-cra-exports
*/

#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "MeanReversionStrategy.h"

#include "FillInfo.h"
#include "AllEventMsg.h"
#include "ExecutionTypes.h"
#include <Utilities/Cast.h>
#include <Utilities/utils.h>

#include <math.h>
#include <iostream>
#include <cassert>
#include <utility>
#include <string>
using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

using namespace std;

MeanReversionStrategy::MeanReversionStrategy(StrategyID strategyID,
const std::string& strategyName, const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    m_momentum_map(),
    m_instrument_order_id_map(),
    m_momentum(0),
    m_aggressiveness(0),
    m_position_size(100),
    m_debug_on(true),
    m_short_window_size(10),
    m_long_window_size(30),
    buy_threshold(0.0001),
    sell_threshold(0.0001),
    window_size(1000),
    current_position(0),
    order_size(100) {
}

MeanReversionStrategy::~MeanReversionStrategy() {
    /* Destry the map. */
    for (const auto& pair : tick_mean_map) {
        Analytics::ScalarRollingWindow<double> current_tick = pair.second;
        current_tick.clear();
    }
}

void MeanReversionStrategy::OnResetStrategyState() {
    m_momentum_map.clear();
    m_instrument_order_id_map.clear();
    m_momentum = 0;
}

void MeanReversionStrategy::DefineStrategyParams() {
}

void MeanReversionStrategy::DefineStrategyCommands() {
}

void MeanReversionStrategy::RegisterForStrategyEvents(
    StrategyEventRegister* eventRegister, DateType currDate) {
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 1);
    }
}

void MeanReversionStrategy::OnTrade(const TradeDataEventMsg& msg) {
    // Trade currentTrade = msg.trade();
    // order_size = currentTrade.size();
    std::string symbol = msg.instrument().symbol();
    double price = msg.trade().price();
    std::cout << "OnTrade(): (" << msg.adapter_time() << "): "
    << msg.instrument().symbol() << ": " << msg.trade().size()
    << " @ $" << msg.trade().price() << std::endl;
    if (tick_mean_map.find(symbol) != tick_mean_map.end()) {
        /* The tick already exists */
        if (tick_mean_map.at(symbol).empty()) {
            /* Initially, just buy some to get started. */
            this->SendSimpleOrder(&msg.instrument(), order_size);
            current_position += order_size;
        } else {
            double mean = tick_mean_map.at(symbol).Mean();
            if ((price - mean) / mean >= sell_threshold) {
                if (current_position >= order_size) {
                /* If the price is higher than mean, sell. */
                    this->SendSimpleOrder(&msg.instrument(), -1 * order_size);
                    current_position -= order_size;
                    // std::cout << "SOLD!" << std::endl;
                } else {
                    /* If price is higher than mean but we don't positions,
                        buy. */
                    this->SendSimpleOrder(&msg.instrument(), order_size);
                    current_position += order_size;
                    // std::cout << "BOUGHT!" << std::endl;
                }
            } else if ((mean - price) / mean >= buy_threshold) {
                if ((mean - price) / mean >= 0.001) {
                    /* Stop the loss. */
                    this->SendSimpleOrder(&msg.instrument(), -1 * order_size);
                    current_position -= order_size;
                    // std::cout << "SELL!" << std::endl;
                } else {
                    /* Else if the price is lower than mean and safe, buy. */
                    this->SendSimpleOrder(&msg.instrument(), order_size);
                    current_position += order_size;
                    // std::cout << "BOUGHT!" << std::endl;
                }
            } else {
                /* The current price does not support to make a trade action.*/
            }
        }
    } else {
        /* The instrument does not exists in means map, need initialization.*/
        Analytics::ScalarRollingWindow<double> new_map(window_size);
        tick_mean_map.insert(std::pair<std::string,
        Analytics::ScalarRollingWindow<double>>(symbol, new_map));
        /* BUY to get started.*/
        this->SendSimpleOrder(&msg.instrument(), 100);
        current_position += 100;
    }
    /* Push to the rolling window. */
    tick_mean_map.at(symbol).push_back(price);
}

void MeanReversionStrategy::OnBar(const BarEventMsg& msg) {
    return;
    // std::cout << "INTO ON BAR() \n";
    if (m_debug_on) {
        ostringstream str;
        str << "FINDME" << msg.instrument().symbol() << ": " << msg.bar();
        logger().LogToClient(LOGLEVEL_DEBUG, str.str().c_str());
        // std::cout << str.str().c_str() << std::endl;
    }

    if (msg.bar().close() < .01) return;
    MomentumMapIterator iter = m_momentum_map.find(&msg.instrument());
    if (iter != m_momentum_map.end()) {
        m_momentum = &iter->second;
    } else {
        m_momentum = &m_momentum_map.insert(make_pair(&msg.instrument(),
        Momentum(m_short_window_size, m_long_window_size))).first->second;
    }

    DesiredPositionSide side = m_momentum->Update(msg.bar().close());

    if (m_momentum->FullyInitialized()) {
        AdjustPortfolio(&msg.instrument(), m_position_size * side);
    }
}

void MeanReversionStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg) {
    std::cout << "OnOrderUpdate(): " <<
    msg.update_time() << msg.name() << std::endl;
    if (msg.completes_order()) {
        m_instrument_order_id_map[msg.order().instrument()] = 0;
        std::cout << "OnOrderUpdate(): order is complete; " << std::endl;
    }
}

void MeanReversionStrategy::AdjustPortfolio(const Instrument* instrument,
int desired_position) {
    int trade_size = desired_position - portfolio().position(instrument);
    if (trade_size != 0) {
        OrderID order_id = m_instrument_order_id_map[instrument];
        // if we're not working an order for the instrument already,
        // place a new order
        if (order_id == 0) {
            SendOrder(instrument, trade_size);
        } else {
            const Order* order = orders().find_working(order_id);
            if (order && ((IsBuySide(order->order_side()) && trade_size < 0) ||
            ((IsSellSide(order->order_side()) && trade_size > 0)))) {
                trade_actions()->SendCancelOrder(order_id);
            }
        }
    }
}

void MeanReversionStrategy::SendSimpleOrder(const Instrument* instrument,
int trade_size) {
    // send order two pennies more aggressive than BBO
    m_aggressiveness = 0.02;
    double last_trade_price = instrument->last_trade().price();
    double price = trade_size > 0 ? last_trade_price + m_aggressiveness :
    last_trade_price - m_aggressiveness;
    OrderParams params(*instrument,
        abs(trade_size),
        price,
        (instrument->type() == INSTRUMENT_TYPE_EQUITY) ? MARKET_CENTER_ID_IEX :
        ((instrument->type() == INSTRUMENT_TYPE_OPTION) ?
        MARKET_CENTER_ID_CBOE_OPTIONS : MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    std::cout << "SendSimpleOrder(): about to send new order for " <<
     trade_size << " at $" << price << std::endl;
    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        m_instrument_order_id_map[instrument] = params.order_id;
        std::cout << "SendOrder(): Sending new order successful!" << std::endl;
    } else {
        std::cout << "SendOrder(): Error sending new order!!!" << tra
        << std::endl;
    }
}


void MeanReversionStrategy::SendOrder(const Instrument* instrument,
int trade_size) {
    return;
}

void MeanReversionStrategy::RepriceAll() {
    for (IOrderTracker::WorkingOrdersConstIter ordit =
    orders().working_orders_begin(); ordit != orders().working_orders_end();
     ++ordit) {
        Reprice(*ordit);
    }
}

void MeanReversionStrategy::Reprice(Order* order) {
    OrderParams params = order->params();
    params.price = (order->order_side() == ORDER_SIDE_BUY) ?
    order->instrument()->top_quote().bid() + m_aggressiveness :
    order->instrument()->top_quote().ask() - m_aggressiveness;
    trade_actions()->SendCancelReplaceOrder(order->order_id(), params);
}

void MeanReversionStrategy::OnStrategyCommand(
    const StrategyCommandEventMsg& msg) {
    switch (msg.command_id()) {
        case 1:
            RepriceAll();
            break;
        case 2:
            trade_actions()->SendCancelAll();
            break;
        default:
            logger().LogToClient(LOGLEVEL_DEBUG, "Unknown command received");
            break;
    }
}

void MeanReversionStrategy::OnParamChanged(StrategyParam& param) {
}
