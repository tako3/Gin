#pragma once

class GinProcessor;
//==============================================================================
class Parameter : public AudioProcessorParameter,
                  protected Timer,
                  protected RealtimeAsyncUpdater
{
public:
    using Ptr = Parameter*;
    
    Parameter (GinProcessor&, String uid, String name, String shortName, String label,
               float minValue, float maxValue,
               float intervalValue, float defaultValue, float skewFactor = 1.0f,
               std::function<String (const Parameter&, float)> textFunction = nullptr);

    Parameter (GinProcessor&, String uid, String name, String shortName, String label,
               NormalisableRange<float> range, float defaultValue,
               std::function<String (const Parameter&, float)> textFunction = nullptr);

    String getUid() { return uid; }
    
    virtual void prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)    {}
    virtual void reset()                                                           {}
    
    //==============================================================================
    virtual float getProcValue (int stepSize);
    
    float getUserValue() const;
    int getUserValueInt() const;
    float getUserDefaultValue() const;
    virtual void setUserValue (float v);
    virtual void setUserValueNotifingHost (float f);
    void setUserValueAsUserAction (float f);
    String getUserValueText() const;
    String userValueToText (float val);

    bool isOnOff();
    bool isOn();

    std::function<float (float)> conversionFunction;
    
    virtual bool isSmoothingActive()            { return false;         }
    
    //==============================================================================
    void beginUserAction();
    void beginUserTimedAction();
    void endUserAction();

    //==============================================================================
    NormalisableRange<float> getUserRange() const { return range;         }
    float getUserRangeStart() const               { return range.start;   }
    float getUserRangeEnd() const                 { return range.end;     }

    //==============================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void parameterChanged (Parameter* param) = 0;
    };

    void addListener (Listener* listener);
    void removeListener (Listener* listener);

    //==============================================================================
    struct ParamState
    {
        String uid;
        float value;
    };

    ParamState getState();
    void setState(const ParamState& state);

    //==============================================================================
    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;

    String getName (int maximumStringLength) const override;
    String getShortName() const;
    String getLabel() const override;

    int getNumSteps() const override;
    String getText (float value, int /*maximumStringLength*/) const override;
    float getValueForText (const String& text) const override;

    bool isOrientationInverted() const override;
    bool isAutomatable() const override;
    bool isMetaParameter() const override;

    float getSkew() { return range.skew; }

protected:
    GinProcessor& processor;
    
    //==============================================================================
    void handleAsyncUpdate() override;
    void timerCallback() override;

    //==============================================================================
    NormalisableRange<float> range;

    float value;
    float defaultValue;
    
    String uid;
    String name;
    String shortName;
    String label;

    std::function<String (const Parameter&, float)> textFunction;

    int userActionCount {0};

    ListenerList<Listener> listeners;
};
