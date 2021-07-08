/*
;    Project:       Open Vehicle Monitor System
;    Date:          8th March 2021
;
;    Changes:
;    1.0  Initial release
;
;    (C) 2021       Shane Hunns
;    (C) 2021       Peter Harry
;    (C) 2021       Michael Balzer
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
*/

#include "ovms_log.h"
static const char *TAG = "v-maxed3";

#include <stdio.h>
#include "vehicle_med3.h"
#include "med3_pids.h"
#include "ovms_webserver.h"
#include <algorithm>
#include "metrics_standard.h"

// Vehicle states:
#define STATE_OFF             0     // Pollstate 0 - POLLSTATE_OFF      - car is off
#define STATE_ON              1     // Pollstate 1 - POLLSTATE_ON       - car is on
#define STATE_RUNNING         2     // Pollstate 2 - POLLSTATE_RUNNING  - car is driving
#define STATE_CHARGING        3     // Pollstate 3 - POLLSTATE_CHARGING - car is charging

// RX buffer access macros: b=byte#
#define RXB_BYTE(b)           m_rxbuf[b]
#define RXB_UINT16(b)         (((uint16_t)RXB_BYTE(b) << 8) | RXB_BYTE(b+1))
#define RXB_UINT24(b)         (((uint32_t)RXB_BYTE(b) << 16) | ((uint32_t)RXB_BYTE(b+1) << 8) \
                               | RXB_BYTE(b+2))
#define RXB_UINT32(b)         (((uint32_t)RXB_BYTE(b) << 24) | ((uint32_t)RXB_BYTE(b+1) << 16) \
                               | ((uint32_t)RXB_BYTE(b+2) << 8) | RXB_BYTE(b+3))
#define RXB_INT8(b)           ((int8_t)RXB_BYTE(b))
#define RXB_INT16(b)          ((int16_t)RXB_UINT16(b))
#define RXB_INT32(b)          ((int32_t)RXB_UINT32(b))

namespace
{

// The parameter namespace for this vehicle
const char PARAM_NAME[] = "xmg";

static const OvmsVehicle::poll_pid_t obdii_polls[] =
    {
        // VCU Polls
//        { vcutx, vcurx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, vcusoc, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //SOC Scaled below
        { vcutx, vcurx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, vcutemp1, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //temp
        { vcutx, vcurx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, vcutemp2, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //temp
        { vcutx, vcurx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, vcuspeed, {  0, 1, 1, 30  }, 0, ISOTP_STD }, //Possible speed 660 - value
        { vcutx, vcurx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, vcupackvolts, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //Pack Voltage
        { vcutx, vcurx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, vcu12vamps, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //12v amps?
        { vcutx, vcurx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, vcuchargervolts, {  0, 15, 15, 15  }, 0, ISOTP_STD }, //charger volts at a guess?
        { vcutx, vcurx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, vcuchargeramps, {  0, 5, 5, 5  }, 0, ISOTP_STD }, //charger amps?
        // BMS Polls
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, cellvolts, {  0, 15, 15, 15  }, 0, ISOTP_STD }, //cell volts
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, celltemps, {  0, 15, 15, 15  }, 0, ISOTP_STD }, //cell temps
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, bmssoc, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //bms SOC
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, bmssoh, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //bms SOH??
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, bmssocraw, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //bms raw SOC
//        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, packamps, {  0, 30, 30, 30  }, 0, ISOTP_STD }, //bms hv amps to pack
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, bmsccsamps, {  0, 15, 15, 15  }, 0, ISOTP_STD }, // looks CCS state
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, bmsacamps, {  0, 15, 15, 15  }, 0, ISOTP_STD }, // looks CCS state
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, bmsccschargeon, {  0, 15, 15, 15  }, 0, ISOTP_STD }, // looks CCS state
        { bmstx, bmsrx, VEHICLE_POLL_TYPE_OBDIIEXTENDED, bmsacchargeon, {  0, 15, 15, 15  }, 0, ISOTP_STD }, // looks CCS state
        { 0, 0, 0x00, 0x00, { 0, 0, 0, 0 }, 0, 0 }
    };
charging_profile granny_steps[]  = {
    {0,98,6375}, // Max charge rate (7200) less charge loss
    {98,100,2700}, // Observed charge rate
    {0,0,0},
    };

charging_profile ccs_steps[] = {
    {0,20,75000},
    {20,30,67000},
    {30,40,59000},
    {40,50,50000},
    {50,60,44000},
    {60,80,37000},
    {80,83,26600},
    {83,95,16200},
    {95,100,5700},
    {0,0,0},
};

}  // anon namespace

// OvmsVehicleMaxed3 constructor
 
OvmsVehicleMaxed3::OvmsVehicleMaxed3()
    {
        ESP_LOGI(TAG, "Start Maxus eDeliver3 vehicle module");

        // Init CAN:
        RegisterCanBus(1,CAN_MODE_ACTIVE,CAN_SPEED_500KBPS);

        // Init BMS:
        BmsSetCellArrangementVoltage(96, 16);
        BmsSetCellArrangementTemperature(16, 1);
        BmsSetCellLimitsVoltage(2.0, 5.0);
        BmsSetCellLimitsTemperature(-39, 200);
        BmsSetCellDefaultThresholdsVoltage(0.020, 0.030);
        BmsSetCellDefaultThresholdsTemperature(2.0, 3.0);

        // Init polling:
        PollSetThrottling(0);
        PollSetResponseSeparationTime(1);
        PollSetState(STATE_OFF);
        PollSetPidList(m_can1, obdii_polls);
        
        // Init VIN:
        memset(m_vin, 0, sizeof(m_vin));
        
        // Init Raw Soc:
        m_poll_state_metric = MyMetrics.InitInt("xmg.state.poll", SM_STALE_MAX, m_poll_state);
        m_soc_raw = MyMetrics.InitFloat("xmg.v.soc.raw", 0, SM_STALE_HIGH, Percentage);
        m_consump_raw = MyMetrics.InitFloat("xmg.v.consump.raw", 0, SM_STALE_HIGH); // temp to monitor bms range
        m_consumprange_raw = MyMetrics.InitFloat("xmg.v.consumprange.raw", 0, SM_STALE_HIGH); // temp to monitor bms range
        m_watt_hour_raw = MyMetrics.InitFloat("xmg.v.watt.hour.raw", 0, SM_STALE_HIGH); // temp to monitor bms range
    
        // Register config params
        MyConfig.RegisterParam("xmg", "Maxus EV configuration", true, true);

        
        // Init Energy:
        StandardMetrics.ms_v_charge_mode->SetValue("standard");
        StandardMetrics.ms_v_env_ctrl_login->SetValue(true);
        med3_cum_energy_charge_wh = 0;
        
        // Init WebServer:
        #ifdef CONFIG_OVMS_COMP_WEBSERVER
            WebInit();
        #endif
    }

//OvmsVehicleMaxed3 destructor

OvmsVehicleMaxed3::~OvmsVehicleMaxed3()
    {
        ESP_LOGI(TAG, "Stop eDeliver3 vehicle module");
        PollSetPidList(m_can1, NULL);
    }

// IncomingPollReply:

void OvmsVehicleMaxed3::IncomingPollReply(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
{
  // init / fill rx buffer:
  if (m_poll_ml_frame == 0) {
    m_rxbuf.clear();
    m_rxbuf.reserve(length + mlremain);
  }
  m_rxbuf.append((char*)data, length);
  if (mlremain)
    return;

  // response complete:
    ESP_LOGV(TAG, "IncomingPollReply: PID %02X: len=%d %s", pid, m_rxbuf.size(), hexencode(m_rxbuf).c_str());
    
    int value1 = (int)data[0];
    int value2 = ((int)data[0] << 8) + (int)data[1];
    
  switch (pid)
  {
      case vcuvin:  // VIN
          StandardMetrics.ms_v_vin->SetValue(m_rxbuf);
          break;
      case bmssoh: //soh
          StandardMetrics.ms_v_bat_soh->SetValue(value1);
          break;
/*      case vcusoc: //soc scaled from 2 - 99
          StandardMetrics.ms_v_bat_soc->SetValue(value1);
          break; */
      case bmssoc: //soc from bms
          StandardMetrics.ms_v_bat_soc->SetValue(value2 / 100.0f);
          if(StandardMetrics.ms_v_bat_soc->AsFloat() >=99.35)StandardMetrics.ms_v_bat_soc->SetValue(100.0f);
          break;
          
      case bmssocraw: //soc from bms
          m_soc_raw->SetValue(value2 / 100.0f);
          break;
      case vcutemp1:  // temperature??
          StandardMetrics.ms_v_mot_temp->SetValue(value1 / 10.0f);
          break;
      case vcutemp2:  // temperature??
          StandardMetrics.ms_v_env_temp->SetValue(value1 / 10.0f);
          break;
      case vcuspeed:  // Possible speed 660 - value with 32 off set
      {
          vanIsOn = StandardMetrics.ms_v_env_on->AsBool();
          if (vanIsOn)
          //StandardMetrics.ms_v_pos_speed->SetValue(628-(value2-32));
          StandardMetrics.ms_v_pos_speed->SetValue(660-(value2));
          //StandardMetrics.ms_v_pos_speed->SetValue((100 - ((value2 - 512) * 100 / (640 - 512))) * 1.60934);
          else
              StandardMetrics.ms_v_pos_speed->SetValue(0);
      }
          break;
      case vcupackvolts:  // Pack Voltage
          StandardMetrics.ms_v_bat_voltage->SetValue(value2 / 10.0f);
          break;

      case bmsccschargeon:
      {
          //StandardMetrics.ms_v_charge_pilot->SetValue(data[0]);
          //StandardMetrics.ms_v_charge_pilot->SetValue(value1);
          bool ccschargeon = value1;
          if (ccschargeon == true) // contains AC on bit
              StdMetrics.ms_v_charge_type->SetValue("ccs");
          //StandardMetrics.ms_v_charge_inprogress->SetValue(true);
      }
          break;
      case bmsccsamps:
        {
            if (StandardMetrics.ms_v_charge_type->AsString() == "ccs")
                StandardMetrics.ms_v_bat_current->SetValue((value2 / 10.0f) - ((value2 / 10.0f) * 2)); // converted to negative
            StandardMetrics.ms_v_bat_power->SetValue((((value2 / 10.0f) - ((value2 / 10.0f) * 2)) * StandardMetrics.ms_v_bat_voltage->AsFloat()) / 1000.0f);// converted to negtive
        }
          break;
      case bmsacchargeon:
        {
            //StandardMetrics.ms_v_charge_pilot->SetValue(value1);
            bool acchargeon = value1;
            if (acchargeon == true) // contains AC on bit
                StdMetrics.ms_v_charge_type->SetValue("type2");
            //StandardMetrics.ms_v_charge_inprogress->SetValue(true);
        }
          break;
      case bmsacamps:
      {
          if (StandardMetrics.ms_v_charge_type->AsString() == "type2")
              StandardMetrics.ms_v_bat_current->SetValue(((value2 / 10.0f) - ((value2 / 10.0f) * 2))); // converted to negative
          StandardMetrics.ms_v_bat_power->SetValue((((value2 / 10.0f) - ((value2 / 10.0f) * 2)) * StandardMetrics.ms_v_bat_voltage->AsFloat()) / 1000.0f);// above converted to negtive
      }
          
          break;
      case vcu12vamps:
          StandardMetrics.ms_v_bat_12v_current->SetValue(value1 / 10.0f);
          break;
      case vcuchargervolts:
          StandardMetrics.ms_v_charge_voltage->SetValue(value2 - 16150); // possible but always 224 untill found only
          break;
      case vcuchargeramps:
          StandardMetrics.ms_v_charge_current->SetValue(value1);
          StandardMetrics.ms_v_charge_climit->SetValue(value1);
          StandardMetrics.ms_v_charge_power->SetValue((StandardMetrics.ms_v_charge_current->AsFloat() * (StandardMetrics.ms_v_charge_voltage->AsFloat() )) / 1000.0f); // work out current untill pid found * (value / 10.0f) / 1000.0f
          break;
      case cellvolts:
        {
            // Read battery cell voltages 1-96:
            BmsRestartCellVoltages();
            for (int i = 0; i < 96; i++)
                {
                    BmsSetCellVoltage(i, RXB_UINT16(i*2) * 0.001f);
                }
            break;
        }
        case celltemps:
        {
            // Read battery cell temperatures 1-16:
                BmsRestartCellTemperatures();
            for (int i = 0; i < 16; i++)
                {
                    BmsSetCellTemperature(i, RXB_UINT16(i*2) * 0.001f);
                }
            break;
        }

    default:
    {
      ESP_LOGW(TAG, "IncomingPollReply: unhandled PID %02X: len=%d %s", pid, m_rxbuf.size(), hexencode(m_rxbuf).c_str());
    }
  }
}

// Can Frames

void OvmsVehicleMaxed3::IncomingFrameCan1(CAN_frame_t* p_frame)
  {
      
//set batt temp
      StandardMetrics.ms_v_bat_temp->SetValue(StandardMetrics.ms_v_bat_pack_tavg->AsFloat());
      
      uint8_t *d = p_frame->data.u8;

      switch (p_frame->MsgID)
        {
        case 0x6f2: // broadcast
          {

          uint8_t meds_status = (d[0] & 0x07); // contains status bits
          
              switch (meds_status)
                  // Pollstate 0 - POLLSTATE_OFF      - car is off
                  // Pollstate 1 - POLLSTATE_ON       - car is on
                  // Pollstate 2 - POLLSTATE_RUNNING  - car is driving
                  // Pollstate 3 - POLLSTATE_CHARGING - car is charging
              break;
          
              
        //Set ideal, est  when CANdata received
              float soc = StandardMetrics.ms_v_bat_soc->AsFloat();
              // Setup Calculates for Efficient Range
                    float batTemp = StandardMetrics.ms_v_bat_temp->AsFloat();
                    float effSoh = StandardMetrics.ms_v_bat_soh->AsFloat();
                    float kmPerKwh = 1000/(consumpRange * 59 + consumpRange) / 60;
                    //float kmPerKwhAvg = (kmPerKwh * 59 + kmPerKwh) / 60;
                    m_watt_hour_raw->SetValue(kmPerKwh);
                    m_consump_raw->SetValue(consumpRange);
                        
                    if(kmPerKwh<4.5)kmPerKwh=4.5;
                    if(kmPerKwh>6.4)kmPerKwh=6.6;
                    if(batTemp>20)batTemp=20;
              
              StandardMetrics.ms_v_bat_range_full->SetValue(241);
              StandardMetrics.ms_v_bat_range_ideal->SetValue(241 * soc / 100);
                //StandardMetrics.ms_v_bat_range_est->SetValue(241 * soc / 108);
              StandardMetrics.ms_v_bat_range_est->SetValue(52.5*((kmPerKwh * (1-((20-batTemp)*1.3)/100)*(soc/100))*(effSoh/100)));
        
              break;
        }
        default:
          break;
                
            case 0x540:  // odometer in KM
                {
                    StandardMetrics.ms_v_pos_odometer->SetValue(d[0] << 16 | d[1] << 8 | d[2]);// odometer
                    break;
                }
      
            case 0x389:  // charger or battery temp
                {
                float invtemp = d[1];
                    StandardMetrics.ms_v_inv_temp->SetValue(invtemp / 10);
                break;
                }
    }
}

void OvmsVehicleMaxed3::Ticker1(uint32_t ticker)
{
    processEnergy();
}

// PollerStateTicker: framework callback: check for state changes
// This is called by VehicleTicker1() just before the next PollerSend().

void OvmsVehicleMaxed3::PollerStateTicker()
{
    bool charging12v = StandardMetrics.ms_v_env_charging12v->AsBool();
    vanIsCharging = StandardMetrics.ms_v_charge_inprogress->AsBool();
    StandardMetrics.ms_v_env_charging12v->SetValue(StandardMetrics.ms_v_bat_12v_voltage->AsFloat() >= 12.9);
    StandardMetrics.ms_v_charge_inprogress->SetValue(-StandardMetrics.ms_v_bat_power->AsFloat() >=  1.000f);
    m_poll_state_metric->SetValue(m_poll_state);
  // Determine new polling state:
  int poll_state;
  if (!charging12v)
    poll_state = STATE_OFF;
  else if (vanIsCharging)
    poll_state = STATE_CHARGING;
  else
    poll_state = STATE_ON;

  // Set base state flags:
  StdMetrics.ms_v_env_awake->SetValue(poll_state == (STATE_ON | STATE_CHARGING));
  StdMetrics.ms_v_env_on->SetValue(poll_state == STATE_ON);

  // Handle polling state change
  if (poll_state == STATE_CHARGING)
  {
    if (m_poll_state != STATE_CHARGING)
        {
        // Charge started:
            if (StandardMetrics.ms_v_charge_type->AsString() == "type2")
            {
            StdMetrics.ms_v_door_chargeport->SetValue(true);
            StdMetrics.ms_v_charge_pilot->SetValue(true);
            //StdMetrics.ms_v_charge_inprogress->SetValue(true);
            //StdMetrics.ms_v_charge_substate->SetValue("onrequest");
            StdMetrics.ms_v_charge_state->SetValue("charging");
            PollSetState(STATE_CHARGING);
            }
            if (StandardMetrics.ms_v_charge_type->AsString() == "ccs")
            {
            StdMetrics.ms_v_door_chargeport->SetValue(true);
            StdMetrics.ms_v_charge_pilot->SetValue(true);
            //StdMetrics.ms_v_charge_inprogress->SetValue(true);
            StdMetrics.ms_v_charge_state->SetValue("charging");
            PollSetState(STATE_CHARGING);
            }
        }
  }
  else
  {
    if (m_poll_state == STATE_CHARGING) {
      // Charge stopped:
      StdMetrics.ms_v_charge_pilot->SetValue(false);
      StdMetrics.ms_v_charge_inprogress->SetValue(false);
      int soc = StdMetrics.ms_v_bat_soc->AsInt();
      if (soc >= 99) {
        StdMetrics.ms_v_charge_substate->SetValue("stopped");
        StdMetrics.ms_v_charge_state->SetValue("done");
      } else {
        StdMetrics.ms_v_charge_substate->SetValue("interrupted");
        StdMetrics.ms_v_charge_state->SetValue("stopped");
      }
    }
  }

  if (poll_state == STATE_ON)
  {
    if (m_poll_state != STATE_ON) {
      // Switched on:
      StdMetrics.ms_v_door_chargeport->SetValue(false);
      StdMetrics.ms_v_charge_substate->SetValue("");
      StdMetrics.ms_v_charge_state->SetValue("");
      PollSetState(STATE_ON);
    }
  }

  if (poll_state == STATE_OFF)
  {
    if (m_poll_state != STATE_OFF) {
      // Switched off:
      StdMetrics.ms_v_bat_current->SetValue(0);
      StdMetrics.ms_v_bat_power->SetValue(0);
      PollSetState(STATE_OFF);
    }
  }
}


// Vehicle framework registration

void OvmsVehicleMaxed3::processEnergy()
{
    // When called each to battery power for a second is calculated.
    // This is added to ms_v_bat_energy_recd if regenerating or
    // ms_v_bat_energy_used if power is being drawn from the battery
    
    // Only calculate if the car is turned on and not charging.
    if (StandardMetrics.ms_v_env_awake->AsBool() &&
        !StandardMetrics.ms_v_charge_inprogress->AsBool()) {
        // Are we in READY state? Ready to drive off.
        if (StandardMetrics.ms_v_env_on->AsBool()) {
            auto bat_power = StandardMetrics.ms_v_bat_power->AsFloat();
            // Calculate battery power (kW) for one second
            auto energy = bat_power / 3600.0;
            // Calculate current (A) used for one second.
            auto coulombs = (StandardMetrics.ms_v_bat_current->AsFloat() / 3600.0);
            // Car is still parked so trip has not started.
            // Set all values to zero
            if (StandardMetrics.ms_v_env_drivetime->AsInt() == 0) {
                ESP_LOGI(TAG, "Trip has started");
                StandardMetrics.ms_v_bat_coulomb_used->SetValue(0);
                StandardMetrics.ms_v_bat_coulomb_recd->SetValue(0);
                StandardMetrics.ms_v_bat_energy_used->SetValue(0);
                StandardMetrics.ms_v_bat_energy_recd->SetValue(0);
            // Otherwise we are already moving so do calculations
            }
            else
            {
                // Calculate regeneration power
                if (bat_power < 0) {
                    StandardMetrics.ms_v_bat_energy_recd->SetValue
                    (StandardMetrics.ms_v_bat_energy_recd->AsFloat() + -(energy));
                    
                    StandardMetrics.ms_v_bat_coulomb_recd->SetValue
                    (StandardMetrics.ms_v_bat_coulomb_recd->AsFloat() + -(coulombs));
                    
                // Calculate power usage. Add power used each second
                }
                else
                {
                    StandardMetrics.ms_v_bat_energy_used->SetValue
                    (StandardMetrics.ms_v_bat_energy_used->AsFloat() + energy);
                    
                    StandardMetrics.ms_v_bat_coulomb_used->SetValue
                    (StandardMetrics.ms_v_bat_coulomb_used->AsFloat() + coulombs);
                }
            }
        }
        // Not in READY so must have been turned off
        else
        {
            // We have only just stopped so add trip values to the totals
            if (StandardMetrics.ms_v_env_parktime->AsInt() == 0) {
                ESP_LOGI(TAG, "Trip has ended");
                StandardMetrics.ms_v_bat_energy_used_total->SetValue
                (StandardMetrics.ms_v_bat_energy_used_total->AsFloat()
                    + StandardMetrics.ms_v_bat_energy_used->AsFloat());
                
                StandardMetrics.ms_v_bat_energy_recd_total->SetValue
                (StandardMetrics.ms_v_bat_energy_recd_total->AsFloat()
                 + StandardMetrics.ms_v_bat_energy_recd->AsFloat());
                
                StandardMetrics.ms_v_bat_coulomb_used_total->SetValue
                (StandardMetrics.ms_v_bat_coulomb_used_total->AsFloat()
                 + StandardMetrics.ms_v_bat_coulomb_used->AsFloat());
                
                StandardMetrics.ms_v_bat_coulomb_recd_total->SetValue
                (StandardMetrics.ms_v_bat_coulomb_recd_total->AsFloat()
                 + StandardMetrics.ms_v_bat_coulomb_recd->AsFloat());
            }
        }
    }
    
    // Add cumulative charge energy each second to ms_v_charge_power
    if(StandardMetrics.ms_v_charge_inprogress->AsBool())
    {
        med3_cum_energy_charge_wh += StandardMetrics.ms_v_charge_power->AsFloat()*1000/3600;
        StandardMetrics.ms_v_charge_kwh->SetValue(med3_cum_energy_charge_wh/1000);
//        med3_cum_energy_charge_wh += StandardMetrics.ms_v_charge_power->AsFloat()/3600;
//        StandardMetrics.ms_v_charge_kwh->SetValue((med3_cum_energy_charge_wh/1000) * 1.609f);
        
        int limit_soc      = StandardMetrics.ms_v_charge_limit_soc->AsInt(0);
        float limit_range    = StandardMetrics.ms_v_charge_limit_range->AsFloat(0, Kilometers);
        float max_range      = StandardMetrics.ms_v_bat_range_full->AsFloat(0, Kilometers);
        
        // Calculate time to reach 100% charge
        int minsremaining_full = StandardMetrics.ms_v_charge_type->AsString() == "ccs" ? calcMinutesRemaining(100, ccs_steps) : calcMinutesRemaining(100, granny_steps);
        StandardMetrics.ms_v_charge_duration_full->SetValue(minsremaining_full);
        ESP_LOGV(TAG, "Time remaining: %d mins to full", minsremaining_full);
        
        // Calculate time to charge to SoC Limit
        if (limit_soc > 0)
        {
            // if limit_soc is set, then calculate remaining time to limit_soc
            int minsremaining_soc = StandardMetrics.ms_v_charge_type->AsString() == "ccs" ? calcMinutesRemaining(limit_soc, ccs_steps) : calcMinutesRemaining(limit_soc, granny_steps);
            StandardMetrics.ms_v_charge_duration_soc->SetValue(minsremaining_soc, Minutes);
            if ( minsremaining_soc == 0 && !soc_limit_reached && limit_soc >= StandardMetrics.ms_v_bat_soc->AsFloat(100))
            {
                  MyNotify.NotifyStringf("info", "charge.limit.soc", "Charge limit of %d%% reached", limit_soc);
                  soc_limit_reached = true;
            }
            ESP_LOGV(TAG, "Time remaining: %d mins to %d%% soc", minsremaining_soc, limit_soc);
        }
        
        // Calculate time to charge to Range Limit
        if (limit_range > 0.0 && max_range > 0.0)
        {
            // if range limit is set, then compute required soc and then calculate remaining time to that soc
            int range_soc = limit_range / max_range * 100.0;
            int minsremaining_range = StandardMetrics.ms_v_charge_type->AsString() == "ccs" ? calcMinutesRemaining(range_soc, ccs_steps) : calcMinutesRemaining(range_soc, granny_steps);
            StandardMetrics.ms_v_charge_duration_range->SetValue(minsremaining_range, Minutes);
            if ( minsremaining_range == 0 && !range_limit_reached && range_soc >= StandardMetrics.ms_v_bat_soc->AsFloat(100))
            {
                MyNotify.NotifyStringf("info", "charge.limit.range", "Charge limit of %dkm reached", (int) limit_range);
                range_limit_reached = true;
            }
            ESP_LOGV(TAG, "Time remaining: %d mins for %0.0f km (%d%% soc)", minsremaining_range, limit_range, range_soc);
        }
        // When we are not charging set back to zero ready for next charge.
    }
    else
    {
        med3_cum_energy_charge_wh=0;
        StandardMetrics.ms_v_charge_duration_full->SetValue(0);
        StandardMetrics.ms_v_charge_duration_soc->SetValue(0);
        StandardMetrics.ms_v_charge_duration_range->SetValue(0);
        soc_limit_reached = false;
        range_limit_reached = false;
    }
}

// Calculate the time to reach the Target and return in minutes
int OvmsVehicleMaxed3::calcMinutesRemaining(int toSoc, charging_profile charge_steps[])
{
    
    int minutes = 0;
    int percents_In_Step;
    int fromSoc = StandardMetrics.ms_v_bat_soc->AsInt(100);
    int batterySize = 52500;
    float chargespeed = -StandardMetrics.ms_v_bat_power->AsFloat() * 1000;

    for (int i = 0; charge_steps[i].toPercent != 0; i++) {
          if (charge_steps[i].toPercent > fromSoc && charge_steps[i].fromPercent<toSoc) {
                 percents_In_Step = (charge_steps[i].toPercent>toSoc ? toSoc : charge_steps[i].toPercent) - (charge_steps[i].fromPercent<fromSoc ? fromSoc : charge_steps[i].fromPercent);
                 minutes += batterySize * percents_In_Step * 0.6 / (chargespeed<charge_steps[i].maxChargeSpeed ? chargespeed : charge_steps[i].maxChargeSpeed);
          }
    }
    return minutes;
}


// Calculate wh/km
void OvmsVehicleMaxed3::calculateEfficiency()
    {
    float consumption = 0;
        if (StandardMetrics.ms_v_pos_speed->AsFloat() >= 5) //temp removed till speed found
        //if (StandardMetrics.ms_v_env_on->AsBool()) //Temp till speed found
            consumption = ABS(StdMetrics.ms_v_bat_power->AsFloat(0, Watts)) / StandardMetrics.ms_v_pos_speed->AsFloat();
    StandardMetrics.ms_v_bat_consumption->SetValue((StandardMetrics.ms_v_bat_consumption->AsFloat() * 29 + consumption) / 30);
        consumpRange = ABS(StandardMetrics.ms_v_bat_consumption->AsFloat());
        
    
    }

//Called by OVMS when a config param is changed
void OvmsVehicleMaxed3::ConfigChanged(OvmsConfigParam* param)
{
    if (param && param->GetName() != PARAM_NAME)
    {
        return;
    }

    ESP_LOGI(TAG, "%s config changed", PARAM_NAME);

    // Instances:
    // xmg
    //  suffsoc              Sufficient SOC [%] (Default: 0=disabled)
    //  suffrange            Sufficient range [km] (Default: 0=disabled)
    StandardMetrics.ms_v_charge_limit_soc->SetValue(
            (float) MyConfig.GetParamValueInt("xmg", "suffsoc"),   Percentage );
    
    if (MyConfig.GetParamValue("vehicle", "units.distance") == "K")
    {
        StandardMetrics.ms_v_charge_limit_range->SetValue(
                (float) MyConfig.GetParamValueInt("xmg", "suffrange"), Kilometers );
    }
    else
    {
        StandardMetrics.ms_v_charge_limit_range->SetValue(
            (float) MyConfig.GetParamValueInt("xmg", "suffrange"), Miles );
    }
}

class OvmsVehicleMaxed3Init
  {
  public: OvmsVehicleMaxed3Init();
  } MyOvmsVehicleMaxed3Init  __attribute__ ((init_priority (9000)));

OvmsVehicleMaxed3Init::OvmsVehicleMaxed3Init()
  {
  ESP_LOGI(TAG, "Registering Vehicle: Maxus eDeliver3 (9000)");

  MyVehicleFactory.RegisterVehicle<OvmsVehicleMaxed3>("MED3","Maxus eDeliver3");
  }
