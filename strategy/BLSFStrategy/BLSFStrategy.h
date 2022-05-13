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

#pragma once

#ifdef _WIN32
    #define _STRATEGY_EXPORTS __declspec(dllexport)
#else
    #ifndef _STRATEGY_EXPORTS
    #define _STRATEGY_EXPORTS
    #endif
#endif

#include <Strategy.h>
#include <Analytics/ScalarRollingWindow.h>
#include <Analytics/InhomogeneousOperators.h>
#include <Analytics/IncrementalEstimation.h>
#include <MarketModels/Instrument.h>
#include <Utilities/ParseConfig.h>

#include <vector>
#include <map>
#include <iostream>
#include <string>

using namespace RCM::StrategyStudio;
using namespace boost::posix_time;
using namespace boost::gregorian;


class BLSFStrategy : public Strategy {
 public:
    enum StrategyState {
        BUY = 0,
        SELL = 1
    };

 public:
    BLSFStrategy(
        StrategyID strategyID,
        const std::string& strategyName,
        const std::string& groupName);
    ~BLSFStrategy();

 public:
    /**
     * This event triggers whenever trade message arrives from a market data source.
     */
    virtual void OnTrade(const TradeDataEventMsg& msg);

    /**
     * This event triggers whenever a Bar interval completes for an instrument
     */ 
    virtual void OnBar(const BarEventMsg& msg);

    /**
     * This event triggers whenever new information arrives about a strategy's orders
     */ 
    virtual void OnOrderUpdate(const OrderUpdateEventMsg& msg);

    /**
    *  Perform reset for strategy state 
    */
    void OnResetStrategyState();

    /**
    * This event triggers whenever a custom strategy command 
    * is sent from the client
    * Needed for the Strategy Studio to create new instance
    */ 
    void OnStrategyCommand(const StrategyCommandEventMsg& msg) {}

    /**
    * Notifies strategy for every succesfull change in the value 
    * of a strategy parameter.
    * Needed for the Strategy Studio to create new instance
    */ 
    void OnParamChanged(StrategyParam& param) {}

/**
 * @brief Helper functions specific to this strategy
 * 
 */
 private:
    void SendQuoteOrder(const Instrument* instrument, int trade_size);
    void SendTradeOrder(const Instrument* instrument, int trade_size);

/**
 * @brief From Strategy
 * 
 */
 private:
    virtual void RegisterForStrategyEvents(
                                        StrategyEventRegister* eventRegister,
                                        DateType currDate);

    virtual void DefineStrategyParams() {}

 private:
    StrategyState currentState;     // Current state of the strategy
    date currentDate;               // Marks the current date
    double prevPrice;
    int prevOrder;
    double totalHold;
};

extern "C" {

    _STRATEGY_EXPORTS const char* GetType() {
        return "BLSFStrategy";
    }

    _STRATEGY_EXPORTS IStrategy* CreateStrategy(
                                                const char* strategyType,
                                                unsigned strategyID,
                                                const char* strategyName,
                                                const char* groupName) {
        if (strcmp(strategyType, GetType()) == 0) {
            return *(new BLSFStrategy(strategyID, strategyName, groupName));
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

    /**
     * @brief Get the Release Version 
     * Used to ensure the strategy was built against a version of the SDK
     * compatible with the server version
     * @return _STRATEGY_EXPORTS const* 
     */
    _STRATEGY_EXPORTS const char* GetReleaseVersion() {
        return Strategy::release_version();
    }
}
