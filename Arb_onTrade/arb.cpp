// Copyright [2022] <UIUC IE498 SP22 Group1>
// Author Yihong Jian, Zihan Zhou

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

    instrucmentSignal(NULL),
    instrucmentTrade(NULL),
    signal("SPY"),    // signal is SPY
    totrade("AAPL"),  // to trade is AAPL

    signalLastPrice {0, 0,0},
    tradeLastPrice {0, 0,0},
    currentState(START),
    quantityHeld(0), lastExePrice(0),
    upThreshold(0.02), downThreshold(0.02)  // parameter to tune 

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

void ArbStrategy::Ontrade(const TradeDataEventMsg& msg) {
    if(debugOn) {
        ostringstream str;
        str << msg.instrument().symbol() << ": " << msg.trade();
        logger().LogToClient(LOGLEVEL_DEBUG, str.str().c_str());
    }
    if (msg.instrument().symbol() == "SPY") {
        if (instrucmentSignal!=NULL){
            if(currentState==START){
                if(msg.trade().price() - min(signalLastPrice[3] ,signalLastPrice[1], signalLastPrice[2]) > upThreshold){
                    currentState = BUY;
                }   
            }
            if (currentState == HOLD){
                if(msg.trade().price() - max(signalLastPrice[3], signalLastPrice[1], signalLastPrice[2]) < - downThreshold){
                    currentState = SELL;
                }
            }
        }
       instrucmentSignal = &msg.instrument();
       signalLastPrice[1] = signalLastPrice[2];
       signalLastPrice[2] = signalLastPrice[3];
       signalLastPrice[3] = msg.trade().price();      
     } 
      //stop-loss
    if (msg.instrument().symbol()!="SPY"){
        if(instrucmentSignal!=NULL){
            if(currentState == HOLD){
                if(msg.trade().price()/lastExePrice < 0.9 || msg.trade().price()/lastExePrice > 1.1){
                    currentState = SELL;
                }
            }
        }    
        instrucmentTrade = &msg.instrument();
        tradeLastPrice[1] = tradeLastPrice[2];
        tradeLastPrice[2] = tradeLastPrice[3];
        tradeLastPrice[3] = msg.trade().price();
    }
    if(currentState == BUY){
        currentState = SENT_BUY;
        SendOrder(instrucmentTrade, msg.trade().size()); 
    }
    if(currentState == SELL){
        currentState = SENT_SELL;
        SendOrder(instrucmentTrade, -1 * quantityHeld); // this->SendOrder?
    }    
}

void ArbStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg)
{    
    if(msg.completes_order()){
        if (currentState == SENT_BUY){
            currentState = HOLD;
            quantityHeld += abs(msg.order().size_completed());
            std::cout << "Update: buy order is complete; size: " << msg.order().size_completed() <<std::endl;
            if (msg.order().size_completed() != 0){
                lastExePrice = msg.order().price();
            }
        }   
        if (currentState == SENT_SELL){
            currentState = START;
            quantityHeld -= abs(msg.order().size_completed());
            std::cout << "Update: sell order is complete; size: " << msg.order().size_completed() <<std::endl;
        }   
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

    int shareX = numToTrade * instBars[instY].close() - portfolio().position(instX);
    int shareY = numToTrade * ratio * instBars[instX].close() - portfolio().position(instY);

    //SendOrder(instX, shareX);
    //SendOrder(instY, shareY);
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
