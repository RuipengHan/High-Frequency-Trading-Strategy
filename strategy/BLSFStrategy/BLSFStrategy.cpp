/**
 * @file BLSFStrategy.h
 * @author Tomoyoshi Kimura
 * @brief Buy last sell first strategy is a strategy were we buy at the end of the day and sell ath the beginning of the day
 * @version 0.1
 * @date 2022-05-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

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

BLSFStrategy::BLSFStrategy(
        StrategyID strategyID,
        const std::string& strategyName,
        const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    currentState(BUY),
    currentDate(0),
    prevPrice(0),
    prevOrder(0),
    totalHold(0) {
}

BLSFStrategy::~BLSFStrategy() {}

void BLSFStrategy::OnResetStrategyState() {
    currentDate = date(0);
    currentState = BUY;
}

void BLSFStrategy::RegisterForStrategyEvents(
        StrategyEventRegister* eventRegister,
        DateType currDate
        ) {
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 10);
    }
}

void BLSFStrategy::OnTrade(const TradeDataEventMsg& msg) {
    date msg_date = msg.adapter_time().date();
    if (currentState == SELL && msg_date != currentDate) {
        double diff = msg.trade().price() - prevPrice;
        if (diff < 0) {
            return;
        }
        currentDate = msg_date;
        std::cout << "OnTrade() SELL: ("
                    << msg.adapter_time()
                    << "): "
                    << msg.instrument().symbol()
                    << ": " << msg.trade().size()
                    << " @ $"
                    << msg.trade().price()
                    << std::endl;
        this->SendTradeOrder(&msg.instrument(), -1 * totalHold);
        cout << "Switching state to BUY" << endl;
        currentState = BUY;
    } else {
        if (currentDate == date(0)) {
            currentDate = msg_date;
        }
        tm date_tm = to_tm(msg.adapter_time());
        if (currentState == BUY &&
                date_tm.tm_hour == 19 &&
                date_tm.tm_min >= 58) {
            std::cout << "OnTrade() BUY: ("
                        << msg.adapter_time()
                        << "): "
                        << msg.instrument().symbol()
                        << ": "
                        << msg.trade().size()
                        << " @ $"
                        << msg.trade().price()
                        << std::endl;
            this->SendTradeOrder(&msg.instrument(), msg.trade().size());
            prevPrice = (prevPrice * prevOrder + msg.trade().price());
            prevPrice /= (prevOrder + 1);
            prevOrder += 1;
            cout << date_tm.tm_hour << "\t" << date_tm.tm_min << endl;
            cout << "Switching State to SELL" << endl;
            currentState = SELL;
        }
    }
}

void BLSFStrategy::OnBar(const BarEventMsg& msg) {
    return;
    if (msg.bar().close() < 0.01) {
        return;
    }
    date msg_date = msg.bar_time().date();
    if (currentState == SELL && msg_date != currentDate) {
        tm date_tm = to_tm(msg.bar_time());
        if (currentState == BUY && date_tm.tm_hour < 13) {
            return;
        }
        if (date_tm.tm_hour == 13 && date_tm.tm_min < 30) {
            return;
        }
        currentDate = msg_date;
        std::cout << "Sending BAR order: ("
                    << msg.bar_time()
                    << "): "
                    << msg.instrument().symbol()
                    << std::endl;
        this->SendQuoteOrder(&msg.instrument(), -1);
        currentState = BUY;
    } else {
        if (currentDate == date(0)) {
            currentDate = msg_date;
        }
        tm date_tm = to_tm(msg.bar_time());
        if (currentState == BUY &&
            date_tm.tm_hour == 19 &&
            date_tm.tm_min >= 58) {
            std::cout << "Sending BAR order: ("
                        << msg.bar_time()
                        << "): "
                        << msg.instrument().symbol()
                        << std::endl;
            this->SendQuoteOrder(&msg.instrument(), 1);
            currentState = SELL;
        }
    }
}

void BLSFStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg) {
    std::cout << "OnOrderUpdate(): "
                << msg.update_time()
                << " "
                << msg.name()
                << std::endl;
    if (msg.completes_order()) {
        std::cout << "OnOrderUpdate(): order is complete; " << std::endl;
    }
}

void BLSFStrategy::SendTradeOrder(
        const Instrument* instrument,
        int trade_size) {
    double last_trade_price = instrument->last_trade().price();
    double price = trade_size > 0 ? last_trade_price : last_trade_price;
    OrderParams params(*instrument,
        abs(trade_size),
        price,
        (instrument->type() == INSTRUMENT_TYPE_EQUITY) ? MARKET_CENTER_ID_IEX :
            ((instrument->type() == INSTRUMENT_TYPE_OPTION) ?
            MARKET_CENTER_ID_CBOE_OPTIONS : MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);
    std::cout << "SendTradeOrder(): about to send new order for "
                << trade_size
                << " at $"
                << price
                << std::endl;
    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        if (trade_size > 0) {
            totalHold += trade_size;
        }
        std::cout << "Sending new trade order successful!" << std::endl;
    } else {
        std::cout << "Error sending new trade order..." << tra << std::endl;
    }
}


void BLSFStrategy::SendQuoteOrder(
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
            << " with missing quote data";
        logger().LogToClient(LOGLEVEL_DEBUG, ss.str());
        std::cout << "SendQuoteOrder(): " << ss.str() << std::endl;
        return;
    }

    double price = trade_size > 0 ? instrument->top_quote().bid() :
                                    instrument->top_quote().ask();

    OrderParams params(*instrument,
        abs(trade_size),
        price,
        (instrument->type() == INSTRUMENT_TYPE_EQUITY) ?
                            MARKET_CENTER_ID_NASDAQ :
        ((instrument->type() == INSTRUMENT_TYPE_OPTION) ?
                            MARKET_CENTER_ID_CBOE_OPTIONS :
                            MARKET_CENTER_ID_CME_GLOBEX),
        (trade_size > 0) ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_LIMIT);

    trade_actions()->SendNewOrder(params);
    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        std::cout << "Sending new quote order successful!" << std::endl;
    } else {
        std::cout << "Error sending new quote order!!!" << tra << std::endl;
    }
}
