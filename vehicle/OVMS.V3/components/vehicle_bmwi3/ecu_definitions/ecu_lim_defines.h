
//
// Warning: don't edit - generated by generate_ecu_code.pl processing ../dev/lim_i1.json: LIM 14: Charging interface module
// This generated code  makes it easier to process CANBUS messages from the LIM ecu in a BMW i3
//

#define I3_ECU_LIM_TX                                                0x06F114
#define I3_ECU_LIM_RX                                                0x0614F1

#define I3_PID_LIM_STATUS_CALCVN                                     0x2541
        // Read out Cal-ID (Calibration-ID) and CVN (Calibration Verification Number). (OBD scopes) Byte layout: 20 bytes
        // long 00-15 = STAT_CALID_WERT 16-19 = STAT_CVN_EINH as Hex unit32 in Intel format / Cal-ID (Calibration-ID) und
        // CVN(Calibration Verification Number) auslesen. (OBD-UmfÃ¤nge)   Byte-Layout: 20 Byte lang 00-15 =
        // STAT_CALID_WERT 16-19 = STAT_CVN_EINH als Hex  unit32 im Intel Format

    // Can't yet generate code for STAT_CALID_TEXT of type string[16], at offset 0. But we account for the 16 bytes
        // Read out Cal ID (here the Cal ID must be output as in mode $ 09 (PID $ 04)). / Cal-ID auslesen (hier muss die
        // Cal-ID wie bei Mode $09 (PID $04) ausgegeben werden).

    #define I3_RES_LIM_STAT_CVN_WERT                                 (RXBUF_UINT32(16))
    #define I3_RES_LIM_STAT_CVN_WERT_UNIT                            'HEX'
    #define I3_RES_LIM_STAT_CVN_WERT_TYPE                            unsigned long
        // Read out CVN (here the CVN must be output as in mode $ 09 (PID $ 06)) / CVN auslesen (hier muss die CVN wie
        // bei Mode $09 (PID $06) ausgegeben werden)

// Skipping ZV_LADESTECKER_CN on 0xDB0E which takes arguments

#define I3_PID_LIM_LADESTECKDOSE_TEMPERATUR                          0xDB0F
        // Temperature of the DC charging connection in ° C (China) / Temperatur der DC-Ladeanschluss in Â°C (China)

    #define I3_RES_LIM_STAT_LADESTECKDOSE_TEMP_WERT                  (RXBUF_UINT(0)/10.0f-40.0)
    #define I3_RES_LIM_STAT_LADESTECKDOSE_TEMP_WERT_UNIT             'Â°C'
    #define I3_RES_LIM_STAT_LADESTECKDOSE_TEMP_WERT_TYPE             float
        // Temperature of the DC charging connection in ° C (China) / Temperatur der DC-Ladeanschluss in Â°C (China)

// Skipping LADEN_HOSPITALITY_FUNKTION on 0xDE6B which takes arguments

// Skipping ZV_LADESTECKER on 0xDEF0 which takes arguments

// Skipping ZV_LADEKLAPPE on 0xDEF1 which takes arguments

#define I3_PID_LIM_LADEBEREITSCHAFT_LIM                              0xDEF2
        // Ready to charge (HW line), (1 = yes, 0 = no) sent from LIM to SLE / Ladebereitschaft (HW-Leitung), (1 = ja, 0
        // = nein) vom LIM an SLE gesendet

    #define I3_RES_LIM_STAT_LADEBEREITSCHAFT_LIM                     (RXBUF_UCHAR(0))
    #define I3_RES_LIM_STAT_LADEBEREITSCHAFT_LIM_UNIT                '0/1'
    #define I3_RES_LIM_STAT_LADEBEREITSCHAFT_LIM_TYPE                unsigned char
        // Ready to charge (HW line), (1 = yes, 0 = no) sent from LIM to SLE / Ladebereitschaft (HW-Leitung), (1 = ja, 0
        // = nein) vom LIM an SLE gesendet

// Skipping LED_LADESTATUS on 0xDEF3 which takes arguments

// Skipping LED_SUCHBELEUCHTUNG on 0xDEF4 which takes arguments

#define I3_PID_LIM_PROXIMITY                                         0xDEF5
        // Current status of the proximity / Aktueller Zustand des Proximity

    #define I3_RES_LIM_STAT_STECKER_NR                               (RXBUF_UCHAR(0))
    #define I3_RES_LIM_STAT_STECKER_NR_UNIT                          '0-n'
    #define I3_RES_LIM_STAT_STECKER_NR_TYPE                          unsigned char
        // Condition of the plug / Zustand des Steckers

    #define I3_RES_LIM_STAT_STROMTRAGFAEHIGKEIT_WERT                 (RXBUF_UCHAR(1))
    #define I3_RES_LIM_STAT_STROMTRAGFAEHIGKEIT_WERT_UNIT            'A'
    #define I3_RES_LIM_STAT_STROMTRAGFAEHIGKEIT_WERT_TYPE            unsigned char
        // Current carrying capacity of the connected cable / StromtragfÃ¤higkeit des angeschlossenen Kabels

#define I3_PID_LIM_PILOTSIGNAL                                       0xDEF6
        // current data of the pilot signal about the charging current / aktuelle Daten des Pilotsignals Ã¼ber den
        // Ladestrom

    #define I3_RES_LIM_STAT_PILOT_AKTIV                              (RXBUF_UCHAR(0))
    #define I3_RES_LIM_STAT_PILOT_AKTIV_UNIT                         '0/1'
    #define I3_RES_LIM_STAT_PILOT_AKTIV_TYPE                         unsigned char
        // State of the pilot signal (0 = not active, 1 = active) / Zustand des Pilotsignals (0 = nicht aktiv, 1 = aktiv)

    #define I3_RES_LIM_STAT_PILOT_PWM_DUTYCYCLE_WERT                 (RXBUF_UCHAR(1))
    #define I3_RES_LIM_STAT_PILOT_PWM_DUTYCYCLE_WERT_UNIT            '%'
    #define I3_RES_LIM_STAT_PILOT_PWM_DUTYCYCLE_WERT_TYPE            unsigned char
        // Pulse duty factor PWM pilot signal / TastverhÃ¤ltnis PWM Pilotsignal

    #define I3_RES_LIM_STAT_PILOT_CURRENT_WERT                       (RXBUF_UCHAR(2))
    #define I3_RES_LIM_STAT_PILOT_CURRENT_WERT_UNIT                  'A'
    #define I3_RES_LIM_STAT_PILOT_CURRENT_WERT_TYPE                  unsigned char
        // Current value calculated from the pilot signal / Errechneter Stromwert aus Pilotsignal

    #define I3_RES_LIM_STAT_PILOT_LADEBEREIT                         (RXBUF_UCHAR(3))
    #define I3_RES_LIM_STAT_PILOT_LADEBEREIT_UNIT                    '0/1'
    #define I3_RES_LIM_STAT_PILOT_LADEBEREIT_TYPE                    unsigned char
        // Vehicle ready to charge state (0 = not ready to charge, 1 = ready to charge) / Zustand Ladebereitschaft
        // Fahrzeug (0 = nicht ladebereit, 1 = ladebereit)

    #define I3_RES_LIM_STAT_PILOT_FREQUENZ_WERT                      (RXBUF_UINT(4))
    #define I3_RES_LIM_STAT_PILOT_FREQUENZ_WERT_UNIT                 'Hz'
    #define I3_RES_LIM_STAT_PILOT_FREQUENZ_WERT_TYPE                 unsigned short
        // Frequency of the pilot signal / Frequenz des Pilotsignals

    #define I3_RES_LIM_STAT_PILOT_PEGEL_WERT                         (RXBUF_UCHAR(6)/10.0f)
    #define I3_RES_LIM_STAT_PILOT_PEGEL_WERT_UNIT                    'V'
    #define I3_RES_LIM_STAT_PILOT_PEGEL_WERT_TYPE                    float
        // Pilot signal level / Pegel des Pilotsignals

#define I3_PID_LIM_LADESCHNITTSTELLE_DC_TEPCO                        0xDEF7
        // Status of the batch control lines / Status der Chargecontroll Leitungen

    #define I3_RES_LIM_STAT_CHARGE_CONTROL_1                         (RXBUF_UCHAR(0))
    #define I3_RES_LIM_STAT_CHARGE_CONTROL_1_UNIT                    '0/1'
    #define I3_RES_LIM_STAT_CHARGE_CONTROL_1_TYPE                    unsigned char
        // Charge control status 1 line (0 = not active, 1 = active) / Zustand Charge control 1 Leitung (0 = nicht aktiv,
        // 1 = aktiv)

    #define I3_RES_LIM_STAT_CHARGE_CONTROL_2                         (RXBUF_UCHAR(1))
    #define I3_RES_LIM_STAT_CHARGE_CONTROL_2_UNIT                    '0/1'
    #define I3_RES_LIM_STAT_CHARGE_CONTROL_2_TYPE                    unsigned char
        // Charge control status 2 line (0 = not active, 1 = active) / Zustand Charge control 2 Leitung (0 = nicht aktiv,
        // 1 = aktiv)

    #define I3_RES_LIM_STAT_CHARGE_PERMISSION                        (RXBUF_UCHAR(2))
    #define I3_RES_LIM_STAT_CHARGE_PERMISSION_UNIT                   '0/1'
    #define I3_RES_LIM_STAT_CHARGE_PERMISSION_TYPE                   unsigned char
        // Charge permission line status (0 = not active, 1 = active) / Zustand Charge Permission Leitung (0 = nicht
        // aktiv, 1 = aktiv)

    #define I3_RES_LIM_STAT_LADESTECKER                              (RXBUF_UCHAR(3))
    #define I3_RES_LIM_STAT_LADESTECKER_UNIT                         '0/1'
    #define I3_RES_LIM_STAT_LADESTECKER_TYPE                         unsigned char
        // State of charging plug (0 = not plugged in, 1 = plugged in) / Zustand Ladestecker (0 = nicht gesteckt, 1 =
        // gesteckt)

#define I3_PID_LIM_DC_SCHUETZ_SCHALTER                               0xDEF8
        // Contactor switch status (DC charging) / Status SchÃ¼tzschalter (DC-Laden)

    #define I3_RES_LIM_STAT_DC_SCHUETZ_SCHALTER                      (RXBUF_UCHAR(0))
    #define I3_RES_LIM_STAT_DC_SCHUETZ_SCHALTER_UNIT                 '0-n'
    #define I3_RES_LIM_STAT_DC_SCHUETZ_SCHALTER_TYPE                 unsigned char
        // Contactor switch status (DC charging) / Status SchÃ¼tzschalter (DC-Laden)

#define I3_PID_LIM_DC_SCHUETZ_SPANNUNG_EINGANG                       0xDEF9
        // Voltage at the input of the relay box (contactors) for DC charging / Spannung am Eingang der Relaisbox
        // (SchaltschÃ¼tze) fÃ¼r DC-Laden

    #define I3_RES_LIM_STAT_DC_SCHUETZ_SPANNUNG_EINGANG_WERT         (RXBUF_UINT(0))
    #define I3_RES_LIM_STAT_DC_SCHUETZ_SPANNUNG_EINGANG_WERT_UNIT    'V'
    #define I3_RES_LIM_STAT_DC_SCHUETZ_SPANNUNG_EINGANG_WERT_TYPE    unsigned short
        // Voltage at the input of the relay box (contactors) for DC charging / Spannung am Eingang der Relaisbox
        // (SchaltschÃ¼tze) fÃ¼r DC-Laden

    #define I3_RES_LIM_STAT_DC_SCHUETZ_SPANNUNG_NEGATIV              (RXBUF_UCHAR(2))
    #define I3_RES_LIM_STAT_DC_SCHUETZ_SPANNUNG_NEGATIV_UNIT         '0/1'
    #define I3_RES_LIM_STAT_DC_SCHUETZ_SPANNUNG_NEGATIV_TYPE         unsigned char
        // Detection of a negative voltage (0 = no or positive voltage / 1 = negative voltage) / Erkennung einer
        // negativen Spannung (0 = keine oder positive Spannung / 1 = negative Spannung)

#define I3_PID_LIM_DC_PINABDECKUNG_COMBO                             0xDEFA
        // State of the DC pin cover for combo socket (0 = closed, 1 = open) / Zustand der DC Pinabdeckung bei
        // Combo-Steckdose (0 = geschlossen, 1 = geÃ¶ffnet)

    #define I3_RES_LIM_STAT_DC_PINABDECKUNG                          (RXBUF_UCHAR(0))
    #define I3_RES_LIM_STAT_DC_PINABDECKUNG_UNIT                     '0/1'
    #define I3_RES_LIM_STAT_DC_PINABDECKUNG_TYPE                     unsigned char
        // State of the DC pin cover for combo socket (0 = closed, 1 = open) / Zustand der DC Pinabdeckung bei
        // Combo-Steckdose (0 = geschlossen, 1 = geÃ¶ffnet)
