/*
 * Company: MDA VST
 * Effect Name: MDA Electric Piano
 * Description: A port of the famous MDA ePiano VST
 *
 * This file was auto-generated by Aviate Audio Effect Creator for the Multiverse.
 */
#include "Aviate/EfxPrint.h"
#include "MDAElecPiano.h"

#include "mdaEPiano.h"

using namespace Aviate;

// Some useful aliases
#define audioBlockReceiveReadOnly receiveReadOnly
#define audioBlockReceiveWritable receiveWritable
#define audioBlockAllocate        allocate

namespace MDAVST_MDAElecPiano {

MDAElecPiano::MDAElecPiano()
: AudioStream(NUM_INPUTS, m_inputQueueArray)
{
    // perform any necessary class initialization here
    m_piano = new mdaEPiano(MAX_VOICES);
    m_piano->setProgram(0);
}

MDAElecPiano::~MDAElecPiano()
{
    // perform any necessary clean up here, though destructors are not
    // called on the hardware, only in the simulator.
    if (m_piano) delete m_piano;
}

void MDAElecPiano::m_init()
{
    m_isInit = true;
}

void MDAElecPiano::update(void)
{
    if (!m_enable) { return; }

    AudioBlock* leftOut = audioBlockAllocate();
    if (!leftOut) { return; }
    AudioBlock* rightOut = audioBlockAllocate();
    if (!rightOut) { release(leftOut); return; }

    // zero the leftOut, then use it to report the input peak as zero
    memset(&leftOut->data[0], 0, AUDIO_SAMPLES_PER_BLOCK * sizeof(AudioDataType));

    if (m_bypass) {
        // also zero the right, then transmit and release both
        memset(&rightOut->data[0], 0, AUDIO_SAMPLES_PER_BLOCK * sizeof(AudioDataType));
        transmit(leftOut, 0);
        transmit(rightOut, 1);
        release(leftOut);
        release(rightOut);
        return;
    }
    m_updateInputPeak(leftOut);

    // otherwise do normal processing
    m_piano->process(leftOut->data, rightOut->data);

    // // DO AUDIO EFFECT PROCESSING
    // for (unsigned idx=0; idx<AUDIO_SAMPLES_PER_BLOCK; idx++) {
    //     leftOut->data[idx]
    // }

    m_updateOutputPeak(leftOut); // you must call m_upateOutputPeak() at the end of update() before transmit
    transmit(leftOut, 0);
    transmit(rightOut, 1);
    release(leftOut);
    release(rightOut);
}

void MDAElecPiano::volume(float value)
{
    float volDbValue = -40.0f + (value * 50.0f);  // remap the normalized value to represent -40dB to +10dB
    volumeDb(volDbValue);  // AudioEffectFloat has built-in volume function in dB, will set m_volume for us
    m_piano->setVolume(m_volume);

    // Demonstrate the efxLogger.printf() function to print over the USB serial port
    // Note: efxLogger.printf() statements will have no effect in RELEASE builds
    efxLogger.printf("volume change: normalized value: %f  dB value: %f\n", value, volDbValue);
}


void MDAElecPiano::processMidi(int status, int data1, int data2) {
    uint8_t type = status & 0xF0U;
    uint8_t channel = status & 0x0FU;
    //efxLogger.printf("MDAElecPiano::MDAElecPiano(): type:%02X channel:%02X  data1:%d  data2:%d\n", type, channel, data1, data2);

    switch(type) {
    case 0x90U : m_piano->noteOn(data1, data2); break;
    case 0x80U : m_piano->noteOff(data1);       break;
    default :
        efxLogger.printf("MDAElecPiano::MDAElecPiano(): calling processMidiController(), type:%02X channel:%02X  data1:%d  data2:%d\n", type, channel, data1, data2);
        m_piano->processMidiController(data1, data2);
    }
}

void MDAElecPiano::program(float value)
{
    m_program = getUserParamValue(Program_e, value);
    m_progSel = static_cast<unsigned>(m_program);
    if (m_progSel >= NPROGS) { m_progSel = 0; }
    if (m_programMode) {
        m_piano->setProgram(m_progSel);
        efxLogger.printf("MDAElecPiano::program(): program set to %u\n", m_progSel);
    }
}

void MDAElecPiano::mode(float value){
    m_programMode = value ? true : false;

    if (!m_programMode) { // manual mode
        efxLogger.printf("MDAElecPiano::mode(): mode set to MANUAL mode\n");
        m_piano->setProgram(0);  // manual mode overwrites program slot 0
        m_piano->setDecay(m_envelopedecay);
        m_piano->setRelease(m_enveloperelease);
        m_piano->setHardness(m_hardness);
        m_piano->setTreble(m_treble);
        m_piano->setPanTremolo(m_pantremolo);
        m_piano->setPanLFO(m_panlfo);
        m_piano->setVelocitySense(m_velocitysensitivity);
        m_piano->setStereo(m_width);
        m_piano->setPolyphony(m_polyphony);
        m_piano->setTune(m_tune);
        m_piano->setDetune(m_detune);
        m_piano->setOverdrive(m_overdrive);
        
    } else {  // program mode
        // restore defaults
        m_piano->setDecay(0.500f);
        m_piano->setRelease(0.500f);
        m_piano->setHardness(0.500f);
        m_piano->setTreble(0.500f);
        m_piano->setPanTremolo(0.500f);
        m_piano->setPanLFO(0.650f);
        m_piano->setVelocitySense(0.250f);
        m_piano->setStereo(0.500f);
        m_piano->setPolyphony(MAX_VOICES);
        m_piano->setTune(0.500f);
        m_piano->setDetune(0.146f);
        m_piano->setOverdrive(0.000f);
        m_piano->setVolume(0.64616f);
        m_piano->setProgram(m_progSel);
        efxLogger.printf("MDAElecPiano::mode(): mode set to PROGRAM %d\n", m_progSel);
    }
}

void MDAElecPiano::envelopedecay(float value)
{
    m_envelopedecay = value;
    if (!m_programMode) {
        m_piano->setDecay(m_envelopedecay);
        efxLogger.printf("MDAElecPiano::envelopedecay(): decay set to %f\n", m_envelopedecay);
    }
}

void MDAElecPiano::enveloperelease(float value)
{
    m_enveloperelease = value;
    if (!m_programMode) {
        m_piano->setRelease(m_enveloperelease);
        efxLogger.printf("MDAElecPiano::enveloperelease(): release set to %f\n", m_enveloperelease);
    }
}

void MDAElecPiano::width(float value)
{
    m_width = value;
    if (!m_programMode) {
        m_piano->setStereo(m_width);
        efxLogger.printf("MDAElecPiano::width(): width set to %f\n", m_width);
    }
}

void MDAElecPiano::velocitysensitivity(float value)
{
    m_velocitysensitivity = value;
    if (!m_programMode) {
        m_piano->setVelocitySense(m_velocitysensitivity);
        efxLogger.printf("MDAElecPiano::velocitysensitivity(): velocity sensitivity set to %f\n", m_velocitysensitivity);
    }
}

void MDAElecPiano::hardness(float value)
{
    m_hardness = value;
    if (!m_programMode) {
        m_piano->setHardness(m_hardness);
        efxLogger.printf("MDAElecPiano::hardness(): hardness set to %f\n", m_hardness);
    }
}

void MDAElecPiano::treble(float value)
{
    m_treble = value;
    if (!m_programMode) {
        m_piano->setTreble(m_treble);
        efxLogger.printf("MDAElecPiano::treble(): treble set to %f\n", m_treble);
    }
}

void MDAElecPiano::polyphony(float value)
{
    m_polyphony = getUserParamValue(Polyphony_e, value);
    if (!m_programMode) {
        efxLogger.printf("MDAElecPiano::polyphony(): polyphony set to %u\n", m_polyphony);
        m_piano->setPolyphony(m_polyphony);
    }
}

void MDAElecPiano::tune(float value)
{
    m_tune = value;
    if (!m_programMode) {m_piano->setTune(m_tune);
    efxLogger.printf("MDAElecPiano::tune(): tune set to %f\n", m_tune);
    }
}

void MDAElecPiano::detune(float value)
{
    m_detune = value;
    if (!m_programMode) {
        m_piano->setDetune(m_detune);
        efxLogger.printf("MDAElecPiano::detune(): detune set to %f\n", m_detune);
    }
}

void MDAElecPiano::pantremolo(float value)
{
    m_pantremolo = value;
    if (!m_programMode) {
        m_piano->setPanTremolo(m_pantremolo);
        efxLogger.printf("MDAElecPiano::pantremolo(): pan tremolo set to %f\n", m_pantremolo);
    }
}

void MDAElecPiano::panlfo(float value)
{
    m_panlfo = value;
    if (!m_programMode) {
        m_piano->setPanLFO(m_panlfo);
        efxLogger.printf("MDAElecPiano::panlfo(): pan LFO set to %f\n", m_panlfo);
    }
}

void MDAElecPiano::overdrive(float value)
{
    m_overdrive = value;
    if (!m_programMode) {
        efxLogger.printf("MDAElecPiano::overdrive(): overdrive set to %f\n", m_overdrive);
        m_piano->setOverdrive(m_overdrive);
    }
    
}

}
