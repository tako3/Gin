/*
 ==============================================================================

 This file is part of the GIN library.
 Copyright (c) 2019 - Roland Rabien.

 ==============================================================================
 */


#pragma once

//==============================================================================
double sine (double phase, double unused1 = 0, double unused2 = 0);
double triangle (double phase, double freq, double sampleRate);
double sawUp (double phase, double freq, double sampleRate);
double sawDown (double phase, double freq, double sampleRate);
double pulse (double phase, double pw, double freq, double sampleRate);
double squareWave (double phase, double freq, double sampleRate);
double noise();

//==============================================================================
class BandLimitedLookupTable
{
public:
    BandLimitedLookupTable (std::function<double (double, double, double)> function, double sampleRate, int notesPerTable = 6, int tableSize = 1024);
    
    void reset (std::function<double (double, double, double)> function, double sampleRate, int notesPerTable = 6, int tableSize = 1024);

    juce::OwnedArray<juce::dsp::LookupTableTransform<float>> tables;

    int notesPerTable = 0;
};

//==============================================================================
enum class Wave
{
    sine,
    triangle,
    sawUp,
    sawDown,
    pulse,
    square,
    noise
};

//==============================================================================
class BandLimitedLookupTables
{
public:
    BandLimitedLookupTables (double sampleRate = 44100);
    
    void setSampleRate (double sampleRate);

    float processSine (float phase);
    float processTriangle (float note, float phase);
    float processSawUp (float note, float phase);
    float processSawDown (float note, float phase);
    float processSquare (float note, float phase);
    float processPulse (float note, float phase, float pw);
    
    float process (Wave wave, float note, float phase, float pw = 0.5f);

private:
    double sampleRate = 0;
    BandLimitedLookupTable sineTable, sawUpTable, sawDownTable, triangleTable;
};

//==============================================================================
class StereoOscillator
{
public:
    StereoOscillator (BandLimitedLookupTables& bllt_) : bllt (bllt_) {}
    
    struct Params
    {
        Wave wave = Wave::sawUp;
        float leftGain = 1.0;
        float rightGain = 1.0;
        float pw = 0.5;
    };
    
    void setSampleRate (double sr)  { sampleRate = sr; }
    void noteOn (float p = -1)      { p >= 0 ? phase = p : Random::getSystemRandom().nextFloat(); }
    
    void process (float note, const Params& params, AudioSampleBuffer& buffer);
    void processAdding (float note, const Params& params, AudioSampleBuffer& buffer);
    
private:
    BandLimitedLookupTables& bllt;
    double sampleRate = 44100.0;
    float phase = 0;
};

//==============================================================================
class VoicedStereoOscillator
{
public:
    VoicedStereoOscillator (BandLimitedLookupTables& bllt, int maxVoices = 8)
    {
        for (int i = 0; i < maxVoices; i++)
            oscillators.add (new StereoOscillator (bllt));
    }
    
    struct Params
    {
        Wave wave = Wave::sawUp;
        int voices = 1;
        float pw = 0.5;
        float pan = 0.0f;
        float spread = 0.0f;
        float detune = 0.0f;
        float gain = 1.0f;
    };
    
    void setSampleRate (double sr)
    {
        for (auto o : oscillators)
            o->setSampleRate (sr);
    }
    
    void noteOn()
    {
        for (auto o : oscillators)
            o->noteOn();
    }
    
    void process (float note, const Params& params, AudioSampleBuffer& buffer)
    {
        buffer.clear();
        processAdding (note, params, buffer);
    }
    
    void processAdding (float note, const Params& params, AudioSampleBuffer& buffer)
    {
        StereoOscillator::Params p;
        p.wave = params.wave;
        p.pw   = params.pw;
        
        if (params.voices == 1)
        {
            p.leftGain  = 1.0f - params.pan;
            p.rightGain = 1.0f + params.pan;

            oscillators[0]->processAdding (note, p, buffer);
        }
        else
        {
            for (int i = 0; i < params.voices; i++)
            {
                float pan = jlimit (-1.0f, 1.0f, ((i % 2 == 0) ? 1 : -1) * params.spread);

                p.leftGain  = params.gain * (1.0f - pan) / params.voices;
                p.rightGain = params.gain * (1.0f + pan) / params.voices;

                float baseNote  = note - params.detune / 2;
                float noteDelta = params.detune / (params.voices - 1);

                oscillators[i]->processAdding (baseNote + noteDelta * i, p, buffer);
            }
        }
    }
    
private:
    OwnedArray<StereoOscillator> oscillators;
};
