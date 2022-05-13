/**
 * @file SwingStrategy.cpp
 * @author Tomoyoshi Kimura
 * @brief 
 * @version 0.1
 * @date 2022-05-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "SwingStrategy.h"

#include "FillInfo.h"
#include "AllEventMsg.h"
#include "ExecutionTypes.h"
#include <Utilities/Cast.h>
#include <Utilities/utils.h>

#include <math.h>
#include <iostream>
#include <cassert>
#include <ctime>
#include <algorithm>

using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

using namespace std;

SwingStrategy::SwingStrategy(
                            StrategyID strategyID,
                            const std::string& strategyName,
                            const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    currentTrend(DESIRED_POSITION_SIDE_FLAT),
    swingMomentum(3, 1000),
    maxSwing(-1),
    minSwing(10000000),
    localMax(-1),
    localMin(10000000),
    beginFlag(true) {
}

SwingStrategy::~SwingStrategy() {}

void SwingStrategy::OnResetStrategyState() {
    maxSwing = -1;
    minSwing = 10000000;
    localMax = -1;
    localMin = 10000000;
}

void SwingStrategy::RegisterForStrategyEvents(
                        StrategyEventRegister* eventRegister,
                        DateType currDate) {
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 10);
    }
}

void SwingStrategy::UpdateLocalSwing(const Bar & bar) {
    if (beginFlag) {
        maxSwing = bar.close();
        minSwing = bar.close();
        beginFlag = false;
    }
    localMax = max(maxSwing, bar.high());
    localMin = min(minSwing, bar.low());
}

void SwingStrategy::UpdateLocalSwing(const Trade & trade) {
    localMax = max(localMax, trade.price());
    localMin = min(localMin, trade.price());
}

DesiredPositionSide SwingStrategy::OrderDecision(const Bar & bar) {
    DesiredPositionSide momentumTrend = swingMomentum.Update(
                                        bar.close(),
                                        currentTrend);
    return momentumTrend;
}

DesiredPositionSide SwingStrategy::OrderDecision(const Trade & trade) {
    DesiredPositionSide momentumTrend = swingMomentum.Update(
                                        trade.price(),
                                        currentTrend);
    if (momentumTrend == currentTrend) {
        return DESIRED_POSITION_SIDE_FLAT;
    }
    return momentumTrend;
}

void SwingStrategy::UpdateSwing() {
    maxSwing = localMax;
    minSwing = localMin;
}

void SwingStrategy::OnTrade(const TradeDataEventMsg & msg) {
    Trade currentTrade = msg.trade();
    if (currentTrade.price() < 0.01) {
        return;
    }

    if (beginFlag) {
        maxSwing = currentTrade.price();
        minSwing = currentTrade.price();
        beginFlag = false;
    }

    UpdateLocalSwing(currentTrade);
    DesiredPositionSide decisionTrend = OrderDecision(currentTrade);
    if (currentTrade.price() > minSwing &&
        currentTrade.price() < maxSwing) {
        currentTrend = DESIRED_POSITION_SIDE_FLAT;
        return;
    }

    if (currentTrade.price() > maxSwing) {
        if (decisionTrend == DESIRED_POSITION_SIDE_SHORT) {
            SendTradeOrder(&msg.instrument(),
                            currentTrade.size() * -1);
            UpdateSwing();
            decisionTrend = DESIRED_POSITION_SIDE_LONG;
        }
    }

    if (currentTrade.price() < minSwing) {
        // cout << "Lower than the min!" << endl;
        if (decisionTrend == DESIRED_POSITION_SIDE_LONG) {
            SendTradeOrder(&msg.instrument(),
                            currentTrade.size() * 1);
            UpdateSwing();
            decisionTrend = DESIRED_POSITION_SIDE_SHORT;
        }
    }
    currentTrend = decisionTrend;
}

void SwingStrategy::OnBar(const BarEventMsg& msg) {
    return;
    Bar currentBar = msg.bar();

    if (currentBar.close() < 0.01) {
        return;
    }

    UpdateLocalSwing(currentBar);
    DesiredPositionSide decisionTrend = OrderDecision(currentBar);

    if (currentBar.close() > minSwing &&
        currentBar.close() < maxSwing) {
        // within the swing, does not exectue
        currentTrend = DESIRED_POSITION_SIDE_FLAT;
        return;
    }

    if (currentBar.close() > maxSwing) {
        if (currentTrend == DESIRED_POSITION_SIDE_SHORT) {
            SendQuoteOrder(&msg.instrument(), 100 * currentTrend);
            UpdateSwing();
        }
    }

    if (currentBar.close() < minSwing) {
        if (currentTrend == DESIRED_POSITION_SIDE_LONG) {
            SendQuoteOrder(&msg.instrument(), 100 * currentTrend);
            UpdateSwing();
        }
    }
}

void SwingStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg) {
    std::cout << "OnOrderUpdate(): "
                << msg.update_time()
                << " " << msg.name()
                << std::endl;
    if (msg.completes_order()) {
        std::cout << "OnOrderUpdate(): order is complete;" << std::endl;
    }
}

void SwingStrategy::SendTradeOrder(
                                    const Instrument* instrument,
                                    int trade_size) {
    double last_trade_price = instrument->last_trade().price();
    double price = trade_size > 0 ? last_trade_price : last_trade_price;

    OrderParams params(*instrument,
        abs(trade_size),
        price,
        (instrument->type() == INSTRUMENT_TYPE_EQUITY)
                        ? MARKET_CENTER_ID_IEX :
        ((instrument->type() == INSTRUMENT_TYPE_OPTION)
                        ? MARKET_CENTER_ID_CBOE_OPTIONS :
                        MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size > 0) ? ORDER_SIDE_BUY :
                            ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    std::cout << "SendTrade(): about to send new order for "
                << trade_size << " at $"
                << price << std::endl;
    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        std::cout << "SendTradeOrder(): Sending new order successful!"
                    << std::endl;
    } else {
        std::cout << "SendTradeOrder(): Error sending new order!!!"
                    << tra
                    << std::endl;
    }
}


void SwingStrategy::SendQuoteOrder(
                                    const Instrument* instrument,
                                    int trade_size) {
    if (instrument->top_quote().ask() < .01 ||
        instrument->top_quote().bid() < .01 ||
        !instrument->top_quote().ask_side().IsValid() ||
        !instrument->top_quote().ask_side().IsValid()) {
        std::stringstream ss;
        ss << "Sending buy order for "
            << instrument->symbol()
            << " at price "
            << instrument->top_quote().ask()
            << " and quantity "
            << trade_size
            <<" with missing quote data";
        logger().LogToClient(LOGLEVEL_DEBUG, ss.str());
        std::cout << "SendQuoteOrder(): " << ss.str() << std::endl;
        return;
    }

    double price = trade_size > 0 ?
                    instrument->top_quote().bid() :
                    instrument->top_quote().ask();

    OrderParams params(*instrument,
        abs(trade_size),
        price,
        (instrument->type() == INSTRUMENT_TYPE_EQUITY) ?
                                MARKET_CENTER_ID_NASDAQ :
        ((instrument->type() == INSTRUMENT_TYPE_OPTION) ?
                                MARKET_CENTER_ID_CBOE_OPTIONS :
                                MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size > 0) ? ORDER_SIDE_BUY :
                        ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    trade_actions()->SendNewOrder(params);
}
