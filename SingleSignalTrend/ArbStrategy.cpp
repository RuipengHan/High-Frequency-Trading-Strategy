// Copyright [2022] <UIUC IE498 SP22 Group1>
// Author Yihong Jian, Zihan Zhou

//#include "ArbStrategy.h"

#ifdef _WIN32
    #include "stdafx.h"
#endif

#include "FillInfo.h"
#include "AllEventMsg.h"
#include "ExecutionTypes.h"
#include <Utilities/Cast.h>
#include <Utilities/utils.h>
#include "ArbStrategy.h"
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
    m_instrument_order_id_map(),
    instX(nullptr),
    instY(nullptr),
    instBars(),
    lastX(0), lastY(0), changeX(0), changeY(0),
    tradeUnit(1),
    ratio(2.0),
    numToTrade(0),

    instrucmentSignal(NULL),
    instrucmentTrade(NULL),
    //instrucmentSignal->last_trade() = trade
    signal("SPY"),    // signal is SPY
    totrade("AAPL"),  // to trade is AAPL
    signalLastPrice {0, 0,0,0,0},
    tradeLastPrice {0, 0,0,0,0},
    currentState(START),
    quantityHeld(0), lastExePrice(0),
    upThreshold(0.00001), downThreshold(0.00001) // parameter to tune
     {
}

ArbStrategy::~ArbStrategy() {
}

void ArbStrategy::DefineStrategyParams() {
    //CreateStrategyParamArgs arg1("trade_unit",
    //    STRATEGY_PARAM_TYPE_STARTUP, VALUE_TYPE_INT, tradeUnit);
    //params().CreateParam(arg1);

  // CreateStrategyParamArgs arg2("debug",
    //    STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_BOOL, debugOn);
    //params().CreateParam(arg2);
}

void ArbStrategy::RegisterForStrategyEvents(
   StrategyEventRegister* eventRegister, DateType currDate) {
  //  (void) currDate;  // suppress compiler warning
 //   for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
   //     EventInstrumentPair regRes =
    //        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 0.1);

   //     if (it == symbols_begin())
   //         instX = regRes.second;
   //     if (it == ++symbols_begin())
  //          instY = regRes.second;
  //  }
}

void ArbStrategy::OnTrade(const TradeDataEventMsg& msg) {
    //std::cout<< msg.instrument().symbol() << ": " << msg.trade();
    //std::cout << "OnTrade(): (" << msg.adapter_time() << "): " << msg.instrument().symbol() << ": " << msg.trade().size() << " @ $" << msg.trade().price() << std::endl;
    //std::cout << "yesmam"<<endl;
   //if((signal=="SPY" && msg.instrument().symbol()=="SPY") || (signal=="AAPL" && msg.instrument().symbol()!="SPY")){

        // Receive new message from SPY: 

        // 1. Apply trade logic
        if(instrucmentSignal!=NULL){
            
            if(currentState==START){
                if(msg.trade().price() - min(signalLastPrice[5],min(signalLastPrice[4],min(signalLastPrice[3],min(signalLastPrice[1], signalLastPrice[2])))) > upThreshold){
                    currentState = BUY;
                }   
            }

            if (currentState == HOLD){
                if(msg.trade().price() - max(signalLastPrice[5],max(signalLastPrice[4],max(signalLastPrice[3],max(signalLastPrice[1], signalLastPrice[2])))) < - downThreshold){
                    currentState = SELL;
                }
                if(msg.trade().price()/lastExePrice < 0.995 || msg.trade().price()/lastExePrice > 1.01){
                   currentState = SELL;
                }
            }
        }       

        // 2. Update historical info
        instrucmentSignal = &msg.instrument();
        signalLastPrice[1] = signalLastPrice[2];
        signalLastPrice[2] = signalLastPrice[3];
        signalLastPrice[3] = signalLastPrice[4];
        signalLastPrice[4] = signalLastPrice[5];
        signalLastPrice[5] = msg.trade().price();
        tradeLastQuantity = msg.trade().size();
    
    
        // stop-loss and take-profit logic
    //    if(instrucmentSignal!=NULL){
   //         if(currentState == HOLD){
    //            if(msg.trade().price()/lastExePrice < 0.995 || msg.trade().price()/lastExePrice > 1.01){
   //                 currentState = SELL;
    //            }
    //        }
  //      }    
  //      instrucmentTrade = &msg.instrument();
 ///       tradeLastPrice[1] = tradeLastPrice[2];
   //     tradeLastPrice[2] = msg.trade().price();
  //      tradeLastQuantity = msg.trade().size();
  //  }
    

    // Execute trade decision
    for (int i=0; i<1; i++){
        if(currentState == BUY){
            currentState = SENT_BUY;
            this->SendSimpleOrder(instrucmentSignal, tradeLastQuantity); //buy multiple shares, the component ticker
        }

        if(currentState == SELL){
            currentState = SENT_SELL;
            this->SendSimpleOrder(instrucmentSignal, -1 * quantityHeld);
        }
    }
}


void ArbStrategy::SendSimpleOrder(const Instrument* instrument, int trade_size)
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

    
    //cout << "Send order. size: " << trade_size << "; price: " << price << "; last trade price: " << last_trade_price << endl;
    TradeActionResult tra = trade_actions()->SendNewOrder(params);
    if (tra == TRADE_ACTION_RESULT_SUCCESSFUL) {
        m_instrument_order_id_map[instrument] = params.order_id;
    }
    else
    {
    	std::cout << "SendOrder(): Error sending new order!!!" << tra << std::endl;
    }

}

void ArbStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg)
{    
      if(msg.completes_order())
    {
		m_instrument_order_id_map[msg.order().instrument()] = 0;

        if (currentState == SENT_BUY){
            currentState = HOLD;
            quantityHeld += abs(msg.order().size_completed());
            std::cout << "Update: buy order is complete; size: " << msg.order().size_completed() <<std::endl;

            if (abs(msg.order().size_completed()) > 0){
                lastExePrice = msg.order().price();
            }
        }   

        if (currentState == SENT_SELL){
            quantityHeld -= abs(msg.order().size_completed());
            std::cout << "Update: sell order is complete; size: " << msg.order().size_completed() <<std::endl;
                currentState = START;
        }   
    }
}

void ArbStrategy::OnBar(const BarEventMsg& msg) {

    //instBars[&msg.instrument()] = msg.bar();

    //if (instBars.size() < 2)
   //     return;

    //if (lastX && lastY) {
   //     changeX = instBars[instX].close() / lastX - 1;
    //    changeY = instBars[instY].close() / lastY - 1;
   // }
   // lastX = instBars[instX].close();
   // lastY = instBars[instY].close();

    // sell X and buy Y when changeX and changeY widens
   // if (changeX > 1.001 * ratio * changeY) {
   //     numToTrade = -tradeUnit;
   // } else if (changeX < -1.001 * ratio * changeY) {
   //     numToTrade = tradeUnit;
   // } else {
   //     numToTrade = 0;
   // }

 //   AdjustPortfolio();

 //   instBars.clear();
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
    std::stringstream ss;
        ss << "Sending order for "
            << instrument->symbol()
            << " at price "
            << instrument->top_quote().ask()
            << " and quantity "
            << trade_size;
        logger().LogToClient(LOGLEVEL_DEBUG, ss.str());
    
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
    //numToTrade = 0;
    //instBars.clear();
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