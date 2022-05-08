// cp SwingStrategy.so ~/Desktop/strategy_studio/backtesting/strategies_dlls/
// create_instance s11w SwingStrategy UIUC SIM-1001-101 dlariviere 10000000 -symbols SPY
// start_backtest 2021-06-01 2021-06-01 name 0
// export_cra_file backtesting-results/BACK_AL1123_2022-05-04_220943_start_06-01-2021_end_06-01-2021.cra backtesting-cra-exports

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

using namespace RCM::StrategyStudio;
using namespace RCM::StrategyStudio::MarketModels;
using namespace RCM::StrategyStudio::Utilities;

using namespace std;

SwingStrategy::SwingStrategy(
                            StrategyID strategyID,
                            const std::string& strategyName,
                            const std::string& groupName):
    Strategy(strategyID, strategyName, groupName),
    priceWindow(NUM_PRICE),
    currentTrend(DESIRED_POSITION_SIDE_FLAT),
    swingMomentum(10, 30),
    maxSwing(-1),
    minSwing(-1),
    localMax(-1),
    localMin(-1) {
}

SwingStrategy::~SwingStrategy(){}

void SwingStrategy::OnResetStrategyState(){}

void SwingStrategy::DefineStrategyParams() {
    CreateStrategyParamArgs arg1("aggressiveness", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_DOUBLE);
    params().CreateParam(arg1);
    
    CreateStrategyParamArgs arg2("debug", STRATEGY_PARAM_TYPE_RUNTIME, VALUE_TYPE_BOOL);
    params().CreateParam(arg2);
}

void SwingStrategy::DefineStrategyCommands() {
    StrategyCommand command1(1, "Reprice Existing Orders");
    commands().AddCommand(command1);

    StrategyCommand command2(2, "Cancel All Orders");
    commands().AddCommand(command2);
}

void SwingStrategy::RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate) { 
    for (SymbolSetConstIter it = symbols_begin(); it != symbols_end(); ++it) {
        eventRegister->RegisterForBars(*it, BAR_TYPE_TIME, 10);
    }
}

void SwingStrategy::UpdateSwing(const Bar & bar) {
    priceWindow.push_back(bar.close());
    maxSwing = max(maxSwing, bar.high());
    minSwing = min(minSwing, bar.low());
}

DesiredPositionSide SwingStrategy::OrderDecision(const Analytics::ScalarRollingWindow <double> & priceWindow) {
    return DESIRED_POSITION_SIDE_FLAT;
}

void SwingStrategy::OnTrade(const TradeDataEventMsg& msg) {	
}

void SwingStrategy::OnBar(const BarEventMsg& msg) {
    Bar currentBar = msg.bar();
    if(currentTrend == DESIRED_POSITION_SIDE_FLAT) {
        UpdateSwing(currentBar);
        // TODO Implement 
        DesiredPositionSide decision = OrderDecision(priceWindow);

        SendOrder(&msg.instrument(), 100 * decision);
        currentTrend = decision;

        if(decision == DESIRED_POSITION_SIDE_LONG) {
            minSwing = currentBar.close();
        } else {
            maxSwing = currentBar.close();
        }
    } 
    else if(currentTrend == DESIRED_POSITION_SIDE_LONG) {
    }
    else {

    }
}

void SwingStrategy::OnOrderUpdate(const OrderUpdateEventMsg& msg)
{    
	std::cout << "OnOrderUpdate(): " << msg.update_time() << " " << msg.name() << std::endl;
    if(msg.completes_order())
    {
		std::cout << "OnOrderUpdate(): order is complete; " << std::endl;
    }
}

void SwingStrategy::AdjustPortfolio(const Instrument* instrument, int desired_position)
{
}

void SwingStrategy::SendSimpleOrder(const Instrument* instrument, int trade_size)
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


void SwingStrategy::SendOrder(const Instrument* instrument, int trade_size)
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

void SwingStrategy::RepriceAll()
{
    for (IOrderTracker::WorkingOrdersConstIter ordit = orders().working_orders_begin(); ordit != orders().working_orders_end(); ++ordit) {
        Reprice(*ordit);
    }
}

void SwingStrategy::Reprice(Order* order)
{
    OrderParams params = order->params();
    params.price = (order->order_side() == ORDER_SIDE_BUY) ? order->instrument()->top_quote().bid() : order->instrument()->top_quote().ask();
    trade_actions()->SendCancelReplaceOrder(order->order_id(), params);
}