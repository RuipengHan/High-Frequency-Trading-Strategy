// Copyright [2022] <UIUC IE498 SP22 Group1>
// Author Yihong Jian

#include "ArbStrategy.h"

#ifdef _WIN32
    #include "stdafx.h"
#endif

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

ArbStrategy::ArbStrategy(StrategyID strategyID,
                    const std::string& strategyName,
                    const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    instX(nullptr),
    instY(nullptr),
    instBars(),
    lastX(0), lastY(0), changeX(0), changeY(0),
    tradeUnit(1),
    ratio(2.0),
    numToTrade(0),
    debugOn(false) {
    instBars.clear();
}

ArbStrategy::~ArbStrategy() {
}

void ArbStrategy::DefineStrategyParams() {
    CreateStrategyParamArgs arg1("trade_unit",
        STRATEGY_PARAM_TYPE_STARTUP, VALUE_TYPE_INT, tradeUnit);
    params().CreateParam(arg1);

    CreateStrategyParamArgs arg2("debug",
        STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_BOOL, debugOn);
    params().CreateParam(arg2);
}

void ArbStrategy::RegisterForStrategyEvents(
    StrategyEventRegister* eventRegister, DateType currDate) {
    (void) currDate;  // suppress compiler warning
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        EventInstrumentPair regRes =
            eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 0.1);

        if (it == symbols_begin())
            instX = regRes.second;
        if (it == ++symbols_begin())
            instY = regRes.second;
    }
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

    int shareX = numToTrade * instBars[instY].close()
            - portfolio().position(instX);
    int shareY = numToTrade * ratio * instBars[instX].close()
            - portfolio().position(instY);

    SendOrder(instX, shareX);
    SendOrder(instY, shareY);
}

void ArbStrategy::SendOrder(const Instrument* instrument, int trade_size) {
    if (debugOn) {
        std::stringstream ss;
        ss << "Sending order for "
            << instrument->symbol()
            << " at price "
            << instrument->top_quote().ask()
            << " and quantity "
            << trade_size;
        logger().LogToClient(LOGLEVEL_DEBUG, ss.str());
    }

    OrderParams params(*instrument,
        abs(trade_size),
        (instrument->top_quote().ask() != 0) ?
            instrument->top_quote().ask() : instrument->last_trade().price(),
        MARKET_CENTER_ID_IEX,
        trade_size > 0 ? ORDER_SIDE_BUY : ORDER_SIDE_SELL,
        ORDER_TIF_DAY,
        ORDER_TYPE_MARKET);

    trade_actions()->SendNewOrder(params);
}

void ArbStrategy::OnResetStrategyState() {
    numToTrade = 0;
    instBars.clear();
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
