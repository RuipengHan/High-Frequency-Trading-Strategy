// Copyright [2022] <UIUC IE498 SP22 Group1>
// Author Zihan Zhou, Yihong Jian

#pragma once

#ifndef _STRATEGY_STUDIO_LIB_EXAMPLES_SIMPLE_MOMENTUM_STRATEGY_H_
#define _STRATEGY_STUDIO_LIB_EXAMPLES_SIMPLE_MOMENTUM_STRATEGY_H_

#ifdef _WIN32
    #define _STRATEGY_EXPORTS __declspec(dllexport)
#else
    #ifndef _STRATEGY_EXPORTS
    #define _STRATEGY_EXPORTS
    #endif
#endif

#include <Strategy.h>
#include <MarketModels/Instrument.h>

#include <string>
#include <unordered_map>
#include <iostream>

using namespace RCM::StrategyStudio;
using std::string;
using std::unordered_map;




class ArbStrategy : public Strategy {
 public:
    ArbStrategy(StrategyID strategyID,
        const std::string& strategyName,
        const std::string& groupName);
    ~ArbStrategy();


 public: /* from IEventCallback */
    /**
     * This event triggers whenever a Bar interval completes for an instrument
     */ 
    virtual void OnBar(const BarEventMsg& msg);


    /**
     * This event triggers whenever a signal trade trend is detected
     */ 
    virtual void OnTrade(const TradeDataEventMsg& msg);

    /**
     * This function detect completed orders and compute quantityHeld
     */ 
    void OnOrderUpdate(const OrderUpdateEventMsg& msg);
    /**
     * 
     *  Perform additional reset for strategy state 
     */
    void OnResetStrategyState();

    /**
     * Notifies strategy for every succesfull change in the value of a strategy parameter.
     *
     * Will be called any time a new parameter value passes validation, including during strategy initialization when default parameter values
     * are set in the call to CreateParam and when any persisted values are loaded. Will also trigger after OnResetStrategyState
     * to remind the strategy of the current parameter values.
     */ 
    void OnParamChanged(StrategyParam& param);

 private:  // Helper functions specific to this strategy
    void AdjustPortfolio();
    void SendOrder(const Instrument* instrument, int trade_size);

 private: /* from Strategy */
    virtual void RegisterForStrategyEvents(
        StrategyEventRegister* eventRegister,
        DateType currDate);

    /**
     * Define any params for use by the strategy 
     */     
    virtual void DefineStrategyParams();

 private:
    // instruments and quotes
    const MarketModels::Instrument* instX;
    const MarketModels::Instrument* instY;
    unordered_map<const Instrument*, Bar> instBars;
    double lastX, lastY, changeX, changeY;
    // hyper params
    int tradeUnit;
    double ratio;
    // number of stock we want to trade
    int numToTrade;
    // debugging flag
    bool debugOn;

 private:
    // instruments and quotes
    const MarketModels::Instrument* instrucmentSignal;
    const MarketModels::Instrument* instrucmentTrade;
     double m_aggressiveness;
    double signalLastPrice[3];     // price container for signal ticker
    double lastExePrice;
    // hyper params:
    double upThreshold;
    double downThreshold;
    // string signal:
    string signal;       // signal ticker: SPY
    string totrade;        // trade ticker: AAPL
    // StrategyState currentState
    double currentState;
    double quantityHeld;
};

extern "C" {

    _STRATEGY_EXPORTS const char* GetType() {
        return "ArbStrategy";
    }

    _STRATEGY_EXPORTS IStrategy* CreateStrategy(const char* strategyType,
                                   unsigned strategyID,
                                   const char* strategyName,
                                   const char* groupName) {
        if (strcmp(strategyType, GetType()) == 0) {
            return *(new ArbStrategy(strategyID, strategyName, groupName));
        } else {
            return NULL;
        }
    }

    // must match an existing user within the system
    _STRATEGY_EXPORTS const char* GetAuthor() {
        return "dlariviere";
    }

    // must match an existing trading group within the system
    _STRATEGY_EXPORTS const char* GetAuthorGroup() {
        return "UIUC";
    }

    // used to ensure the strategy was built against a version of
    // the SDK compatible with the server version
    _STRATEGY_EXPORTS const char* GetReleaseVersion() {
        return Strategy::release_version();
    }
}

#endif
