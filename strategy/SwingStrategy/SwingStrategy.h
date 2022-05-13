/**
 * @file SwingStrategy.h
 * @author Tomoyoshi (Tommy Kimura)
 * @brief 
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

#define NUM_PRICE 20


enum DesiredPositionSide {
    DESIRED_POSITION_SIDE_SHORT = -1,
    DESIRED_POSITION_SIDE_FLAT = 0,
    DESIRED_POSITION_SIDE_LONG = 1
};

class Momentum {
 public:
    Momentum(
        int short_window_size = 10,
        int long_window_size = 30):
        m_shortWindow(short_window_size),
        m_longWindow(long_window_size) {
    }

    void Reset() {
        m_shortWindow.clear();
        m_longWindow.clear();
    }

    DesiredPositionSide Update(double val, DesiredPositionSide currentTrend) {
        double prev_back = m_shortWindow.back();
        m_shortWindow.push_back(val);
        m_longWindow.push_back(val);
        if (currentTrend == DESIRED_POSITION_SIDE_LONG && prev_back > val) {
            return DESIRED_POSITION_SIDE_FLAT;
        }
        if (currentTrend == DESIRED_POSITION_SIDE_SHORT && prev_back < val) {
            return DESIRED_POSITION_SIDE_FLAT;
        }

        double price_diff = m_shortWindow.Mean() - m_longWindow.Mean();
        if (m_shortWindow.Mean() > m_longWindow.Mean()) {
            if (price_diff / m_longWindow.Mean() < 0.000006) {
                return DESIRED_POSITION_SIDE_LONG;
            }
        } else if (m_shortWindow.Mean() < m_longWindow.Mean()) {
            if (price_diff / m_longWindow.Mean() > -0.000006) {
                return DESIRED_POSITION_SIDE_SHORT;
            }
        }
        return DESIRED_POSITION_SIDE_FLAT;
    }

    bool FullyInitialized() {
        return (m_shortWindow.full() && m_longWindow.full());
    }

    Analytics::ScalarRollingWindow <double> m_shortWindow;
    Analytics::ScalarRollingWindow <double> m_longWindow;
};

class SwingStrategy : public Strategy {
 public:
        SwingStrategy(
            StrategyID strategyID,
            const std::string& strategyName,
            const std::string& groupName);
        ~SwingStrategy();

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
        * This event triggers whenever a custom strategy command is sent from the client
        * Needed for the Strategy Studio to create new instance
        */ 
        void OnStrategyCommand(const StrategyCommandEventMsg& msg) {}

        /**
        * Notifies strategy for every succesfull change in the value of a strategy parameter.
        * Needed for the Strategy Studio to create new instance
        */ 
        void OnParamChanged(StrategyParam& param) {}

 private:
        void SendQuoteOrder(const Instrument* instrument, int trade_size);
        void SendTradeOrder(const Instrument* instrument, int trade_size);
        void UpdateLocalSwing(const Bar & bar);
        void UpdateLocalSwing(const Trade & trade);
        void UpdateSwing();
        DesiredPositionSide OrderDecision(const Bar & bar);
        DesiredPositionSide OrderDecision(const Trade & trade);

 private: /* from Strategy */
        virtual void RegisterForStrategyEvents(
            StrategyEventRegister* eventRegister,
            DateType currDate);
        // Needed for the Strategy Studio to create new instance
        virtual void DefineStrategyParams() {}

 private:
        // Trend/side
        DesiredPositionSide currentTrend;
        // Momentum for trending analysis
        Momentum swingMomentum;
        // Swing status, Max and Low
        double maxSwing;
        double minSwing;
        // Temporal Swing
        double localMax;
        double localMin;
        bool beginFlag;
};

extern "C" {
    _STRATEGY_EXPORTS const char* GetType() {
        return "SwingStrategy";
    }

    _STRATEGY_EXPORTS IStrategy* CreateStrategy(
                                                const char* strategyType,
                                                unsigned strategyID,
                                                const char* strategyName,
                                                const char* groupName) {
        if (strcmp(strategyType, GetType()) == 0) {
            return *(new SwingStrategy(strategyID, strategyName, groupName));
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
     * @brief Get the Release Version object
     * Used to ensure the strategy was built
     * against a version of the SDK compatible with the server version
     * @return _STRATEGY_EXPORTS const* 
     */
    _STRATEGY_EXPORTS const char* GetReleaseVersion() {
        return Strategy::release_version();
    }
}
