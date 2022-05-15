// Copyright [2022] <UIUC IE498 SP22 Group1>
/**
 * @file ArbStratehy.cpp
 * @author Zihan ZHOU, Yihong Jian
 *       //
 *         Yihong has implemented On_bar fuction, but due to lack of quotes data with single data_center, 
 *        ArbStrategy truns to focus onTrade function, Yihong truns to deveop work.
 *        Zihan implements current Arbstrategy and test accordingly functionality.
 * @brief This is a trading strategy that traces two market ticks, using one of the tick(here chosen 'SPY') as a signal tick, 
 *        and conduct its trending analysis to determine buying or selling decisions of the other trading tick.
 * @version 1.0
 * @date 2022-05-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "ArbStrategy.h"

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
#include <algorithm>
using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

using namespace std;

ArbStrategy::ArbStrategy(StrategyID strategyID,
                    const std::string& strategyName,
                    const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    // onbar
    instX(nullptr),
    instY(nullptr),
    instBars(),
    lastX(0), lastY(0), changeX(0), changeY(0),
    tradeUnit(1),
    ratio(2.0),
    numToTrade(0),
    // ontrade
    instrucmentSignal(NULL),
    instrucmentTrade(NULL),
    signal("SPY"),    // signal is SPY
    totrade("AAPL"),  // to trade is AAPL
    signalLastPrice{0, 0, 0},
    currentState(0),
    quantityHeld(0), lastExePrice(0),
    upThreshold(0.05), downThreshold(0.05) {
}

ArbStrategy::~ArbStrategy() {
}

void ArbStrategy::DefineStrategyParams() {
}

void ArbStrategy::RegisterForStrategyEvents(
    StrategyEventRegister* eventRegister, DateType currDate) {
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 1);
    }
}

void ArbStrategy::OnTrade(const TradeDataEventMsg& msg) {
     std::cout << "OnTrade(): (" << msg.adapter_time() << "): "
     << msg.instrument().symbol() << ": " << msg.trade().size()
     << " @ $" << msg.trade().price() << std::endl;
     if (msg.instrument().symbol() == "SPY") {
        if (instrucmentSignal != NULL) {
            if (currentState == 0) {
                if (msg.trade().price() - min(signalLastPrice[3],
                min(signalLastPrice[1], signalLastPrice[2])) > upThreshold) {
                    std::cout << "enter_1" << endl;
                    currentState = 2;
                }
            }
            if (currentState == 3) {
                if (downThreshold < msg.trade().price() -
                max(signalLastPrice[3],
                max(signalLastPrice[1], signalLastPrice[2]))) {
                    currentState = 4;
                }
            }
        }
       instrucmentSignal = &msg.instrument();
       signalLastPrice[1] = signalLastPrice[2];
       signalLastPrice[2] = signalLastPrice[3];
       signalLastPrice[3] = msg.trade().price();
     }
      // stop-loss. controlled over 1%
    if (msg.instrument().symbol() != "SPY") {
        if (instrucmentSignal != NULL) {
            if (currentState == 3) {
                if (msg.trade().price()/lastExePrice < 0.99
                || msg.trade().price()/lastExePrice > 1.01) {
                   currentState = 4;
                }
           }
        }
        instrucmentTrade = &msg.instrument();
    }
    if (instrucmentTrade != NULL) {
      if (currentState == 2) {
        currentState = 1;
        SendOrder(instrucmentTrade, msg.trade().size());
        currentState = 3;
        quantityHeld += abs(msg.trade().size());
    }
      if (currentState == 4) {
        currentState = 5;
        SendOrder(instrucmentTrade, -1 * quantityHeld);
        currentState = 0;
        quantityHeld = 0;
     }
    }
}


void ArbStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg) {
}

void ArbStrategy::OnBar(const BarEventMsg& msg) {
    if (debugOn) {
        ostringstream str;
        str << msg.instrument().symbol() << ": "<< msg.bar();
        logger().LogToClient(LOGLEVEL_DEBUG, str.str().c_str());
    }
    instBars[&msg.instrument()] = msg.bar();
    if (instBars.size() < 2)
        return;

    if (lastX && lastY) {
        changeX = instBars[instX].close() / lastX - 1;
        changeY = instBars[instY].close() / lastY - 1;
    }
    lastX = instBars[instX].close();
    lastY = instBars[instY].close();

    // sell X and buy Y when changeX and changeY widens
    if (changeX > 1.001 * ratio * changeY) {
        numToTrade = -tradeUnit;
    } else if (changeX < -1.001 * ratio * changeY) {
        numToTrade = tradeUnit;
    } else {
        numToTrade = 0;
    }
    AdjustPortfolio();
    instBars.clear();
}

void ArbStrategy::AdjustPortfolio() {
    if (orders().num_working_orders() > 0)
        return;
    // int shareX = numToTrade * instBars[instY].close() -
    // portfolio().position(instX);
    // int shareY = numToTrade * ratio * instBars[instX].close() -
    // portfolio().position(instY);
}

void ArbStrategy::SendOrder(const Instrument* instrument, int trade_size) {
    m_aggressiveness = 0.02;  // send order two pennies more aggressive than BBO
    double last_trade_price = instrument->last_trade().price();
    double price = trade_size > 0 ? last_trade_price + m_aggressiveness :
    last_trade_price - m_aggressiveness;
    lastExePrice = price;  //
    OrderParams params(*instrument,
        abs(trade_size),
        price,
        (instrument->type() == INSTRUMENT_TYPE_EQUITY) ? MARKET_CENTER_ID_IEX :
        ((instrument->type() == INSTRUMENT_TYPE_OPTION) ?
        MARKET_CENTER_ID_CBOE_OPTIONS :
        MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    std::cout << "SendSimpleOrder(): about to send new order for " <<
    trade_size << " at $" << price << std::endl;
    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        // std::cout << "SendOrder(): Sending new order successful!" <<
        // std::endl;
    } else {
        // std::cout << "SendOrder(): Error sending new order!!!" << tra <<
        // std::endl;
    }
}

void ArbStrategy::OnResetStrategyState() {
    numToTrade = 0;
}

void ArbStrategy::OnParamChanged(StrategyParam& param) {
    if (param.param_name() == "trade_unit") {
        if (!param.Get(&tradeUnit))
            throw StrategyStudioException("Could not get m_aggressiveness");
    } else if (param.param_name() == "debug") {
        if (!param.Get(&debugOn))
            throw StrategyStudioException("Could not get trade size");
    } else {
        throw StrategyStudioException("Invalid command ");
    }
}
