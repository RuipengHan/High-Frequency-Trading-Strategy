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

using namespace RCM::StrategyStudio;
using namespace boost::posix_time;
using namespace boost::gregorian;

#define NUM_PRICE 20


enum DesiredPositionSide {
    DESIRED_POSITION_SIDE_SHORT=-1,
    DESIRED_POSITION_SIDE_FLAT=0,
    DESIRED_POSITION_SIDE_LONG=1
};

class Momentum {
public:
    
    Momentum(int short_window_size = 10, int long_window_size = 30):
        m_shortWindow(short_window_size), 
        m_longWindow(long_window_size) {

    }

    void Reset() {
        m_shortWindow.clear();
        m_longWindow.clear();
    }

    DesiredPositionSide Update(double val) {

        m_shortWindow.push_back(val);
        m_longWindow.push_back(val);
        if(m_shortWindow.Mean()>m_longWindow.Mean())
            return DESIRED_POSITION_SIDE_LONG;
        else
            return DESIRED_POSITION_SIDE_SHORT;
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
     *  Perform additional reset for strategy state 
     */
    void OnResetStrategyState();
 
    void OnDataSubscription(const DataSubscriptionEventMsg& msg) {}

    void OnStrategyCommand(const StrategyCommandEventMsg& msg) {};

    void OnParamChanged(StrategyParam& param) {};

private: // Helper functions specific to this strategy
    void AdjustPortfolio(const Instrument* instrument, int desired_position);
    void SendOrder(const Instrument* instrument, int trade_size);
    void SendSimpleOrder(const Instrument* instrument, int trade_size);
    void RepriceAll();
    void Reprice(Order* order);
    void UpdateSwing(const Bar & bar);
    DesiredPositionSide OrderDecision(const Analytics::ScalarRollingWindow <double> & priceWindow);

private: /* from Strategy */
    
    virtual void RegisterForStrategyEvents(StrategyEventRegister* eventRegister, DateType currDate); 
    
    /**
     * Define any params for use by the strategy 
     */     
    virtual void DefineStrategyParams();

    /**
     * Define any strategy commands for use by the strategy
     */ 
    virtual void DefineStrategyCommands();

private:
    // price windows
    Analytics::ScalarRollingWindow <double> priceWindow;
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

};

extern "C" {

    _STRATEGY_EXPORTS const char* GetType()
    {
        return "SwingStrategy";
    }

    _STRATEGY_EXPORTS IStrategy* CreateStrategy(const char* strategyType, 
                                   unsigned strategyID, 
                                   const char* strategyName,
                                   const char* groupName)
    {
        if (strcmp(strategyType,GetType()) == 0) {
            return *(new SwingStrategy(strategyID, strategyName, groupName));
        } else {
            return NULL;
        }
    }

     // must match an existing user within the system 
    _STRATEGY_EXPORTS const char* GetAuthor()
    {
        return "dlariviere";
    }

    // must match an existing trading group within the system 
    _STRATEGY_EXPORTS const char* GetAuthorGroup()
    {
        return "UIUC";
    }

    // used to ensure the strategy was built against a version of the SDK compatible with the server version
    _STRATEGY_EXPORTS const char* GetReleaseVersion()
    {
        return Strategy::release_version();
    }
}

