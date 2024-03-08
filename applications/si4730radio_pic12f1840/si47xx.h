#ifndef SI47xx_H
#define SI47xx_H

#define POWER_UP_FM 0  // FM
#define POWER_UP_AM 1  // AM and SSB (if patch applyed)
#define POWER_UP_WB 3  // Weather Band Receiver
#define POWER_PATCH 15 //

// SI473X commands (general)
#define SI473X_ADDR_SEN_LOW 0x11  // SI473X I2C bus address when the SEN pin (16) is set to low 0V.
#define SI473X_ADDR_SEN_HIGH 0x63 // SI473X I2C bus address when the SEN pin (16) is set to high +3.3V

#define POWER_UP 0x01       // Power up device and mode selection.
#define GET_REV 0x10        // Returns revision information on the device.
#define POWER_DOWN 0x11     // Power down device.
#define SET_PROPERTY 0x12   // Sets the value of a property.
#define GET_PROPERTY 0x13   // Retrieves a property’s value.
#define GET_INT_STATUS 0x14 // Read interrupt status bits.

// FM
#define FM_TUNE_FREQ 0x20
#define FM_SEEK_START 0x21 // Begins searching for a valid FM frequency.
#define FM_TUNE_STATUS 0x22
#define FM_AGC_STATUS 0x27
#define FM_AGC_OVERRIDE 0x28
#define FM_RSQ_STATUS 0x23
#define FM_RDS_STATUS 0x24 // Returns RDS information for current channel and reads an entry from the RDS FIFO.

#define FM_NB_DETECT_THRESHOLD 0x1900 // Sets the threshold for detecting impulses in dB above the noise floor. Default value is 16.
#define FM_NB_INTERVAL 0x1901         // Interval in micro-seconds that original samples are replaced by interpolated clean sam- ples. Default value is 24 µs.
#define FM_NB_RATE 0x1902             // Noise blanking rate in 100 Hz units. Default value is 64.
#define FM_NB_IIR_FILTER 0x1903       // Sets the bandwidth of the noise floor estimator Default value is 300.
#define FM_NB_DELAY 0x1904            // Delay in micro-seconds before applying impulse blanking to the original sam- ples. Default value is 133.

// FM RDS properties
#define FM_RDS_INT_SOURCE 0x1500
#define FM_RDS_INT_FIFO_COUNT 0x1501
#define FM_RDS_CONFIG 0x1502
#define FM_RDS_CONFIDENCE 0x1503

#define FM_DEEMPHASIS 0x1100
#define FM_BLEND_STEREO_THRESHOLD 0x1105
#define FM_BLEND_MONO_THRESHOLD 0x1106
#define FM_BLEND_RSSI_STEREO_THRESHOLD 0x1800
#define FM_BLEND_RSSI_MONO_THRESHOLD 0x1801
#define FM_BLEND_SNR_STEREO_THRESHOLD 0x1804
#define FM_BLEND_SNR_MONO_THRESHOLD 0x1805
#define FM_BLEND_MULTIPATH_STEREO_THRESHOLD 0x1808
#define FM_BLEND_MULTIPATH_MONO_THRESHOLD 0x1809
#define FM_CHANNEL_FILTER 0x1102
#define FM_SOFT_MUTE_MAX_ATTENUATION 0x1302

// FM SEEK Properties
#define FM_SEEK_BAND_BOTTOM 0x1400         // Sets the bottom of the FM band for seek
#define FM_SEEK_BAND_TOP 0x1401            // Sets the top of the FM band for seek
#define FM_SEEK_FREQ_SPACING 0x1402        // Selects frequency spacing for FM seek
#define FM_SEEK_TUNE_SNR_THRESHOLD 0x1403  // Sets the SNR threshold for a valid FM Seek/Tune
#define FM_SEEK_TUNE_RSSI_THRESHOLD 0x1404 // Sets the RSSI threshold for a valid FM Seek/Tune


// NBFM Commands
#define NBFM_TUNE_FREQ 0x50
#define NBFM_TUNE_STATUS 0x52
#define NBFM_RSQ_STATUS 0x53
#define NBFM_AGC_STATUS 0x57
#define NBFM_AGC_OVERRIDE 0x58


// NBFM Properties

#define NBFM_MAX_TUNE_ERROR 0x5108
#define NBFM_RSQ_INT_SOURCE 0x5200
#define NBFM_RSQ_SNR_HI_THRESHOLD 0x5201
#define NBFM_RSQ_SNR_LO_THRESHOLD 0x5202
#define NBFM_RSQ_RSSI_HI_THRESHOLD 0x5203
#define NBFM_RSQ_RSSI_LO_THRESHOLD 0x5204
#define NBFM_VALID_SNR_THRESHOLD 0x5403
#define NBFM_VALID_RSSI_THRESHOLD 0x5404


// AM command
#define AM_TUNE_FREQ 0x40    // Tunes to a given AM frequency.
#define AM_SEEK_START 0x41   // Begins searching for a valid AM frequency.
#define AM_TUNE_STATUS 0x42  // Queries the status of the already issued AM_TUNE_FREQ or AM_SEEK_START command.
#define AM_RSQ_STATUS 0x43   // Queries the status of the Received Signal Quality (RSQ) for the current channel.
#define AM_AGC_STATUS 0x47   // Queries the current AGC settings.
#define AM_AGC_OVERRIDE 0x48 // Overrides AGC settings by disabling and forcing it to a fixed value.
#define GPIO_CTL 0x80        // Configures GPO1, 2, and 3 as output or Hi-Z.
#define GPIO_SET 0x81        // Sets GPO1, 2, and 3 output level (low or high).

//SSB command (SAME AM CMD VALUES)
// See AN332 REV 0.8 UNIVERSAL PROGRAMMING GUIDE; pages 4 and 5
#define SSB_TUNE_FREQ 0x40    // Tunes to a given SSB frequency.
#define SSB_TUNE_STATUS 0x42  // Queries the status of the already issued SSB_TUNE_FREQ or AM_SEEK_START command.
#define SSB_RSQ_STATUS 0x43   // Queries the status of the Received Signal Quality (RSQ) for the current channel.
#define SSB_AGC_STATUS 0x47   // Queries the current AGC settings.
#define SSB_AGC_OVERRIDE 0x48 // Overrides AGC settings by disabling and forcing it to a fixed value.

// AM/SW/LW Receiver Property Summary
// See  Si47XX PROGRAMMING GUIDE AN332 (REV 1.0); page 125
#define DIGITAL_OUTPUT_FORMAT 0x0102                // Configure digital audio outputs.
#define DIGITAL_OUTPUT_SAMPLE_RATE 0x0104           // Configure digital audio output sample rate
#define REFCLK_FREQ 0x0201                          //Sets frequency of reference clock in Hz. The range is 31130 to 34406 Hz, or 0 to disable the AFC. Default is 32768 Hz.
#define REFCLK_PRESCALE 0x0202                      // Sets the prescaler value for RCLK input.
#define AM_DEEMPHASIS 0x3100                        // Sets deemphasis time constant. Can be set to 50 µs. Deemphasis is disabled by default.
#define AM_CHANNEL_FILTER 0x3102                    // Selects the bandwidth of the channel filter for AM reception. The choices are 6, 4, 3, 2, 2.5, 1.8, or 1 (kHz). The default bandwidth is 2 kHz.
#define AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN 0x3103 // Sets the maximum gain for automatic volume control.
#define AM_MODE_AFC_SW_PULL_IN_RANGE 0x3104         // Sets the SW AFC pull-in range.
#define AM_MODE_AFC_SW_LOCK_IN_RANGE 0x3105         // Sets the SW AFC lock-in.
#define AM_RSQ_INTERRUPTS 0x3200                    // Same SSB - Configures interrupt related to Received Signal Quality metrics. All interrupts are disabled by default.
#define AM_RSQ_SNR_HIGH_THRESHOLD 0x3201            //Sets high threshold for SNR interrupt.
#define AM_RSQ_SNR_LOW_THRESHOLD 0x3202             // Sets low threshold for SNR interrupt.
#define AM_RSQ_RSSI_HIGH_THRESHOLD 0x3203           // Sets high threshold for RSSI interrupt.
#define AM_RSQ_RSSI_LOW_THRESHOLD 0x3204            // Sets low threshold for RSSI interrupt.
#define AM_SOFT_MUTE_RATE 0x3300                    // Sets the attack and decay rates when entering or leaving soft mute. The default is 278 dB/s.
#define AM_SOFT_MUTE_SLOPE 0x3301                   // Sets the AM soft mute slope. Default value is a slope of 1.
#define AM_SOFT_MUTE_MAX_ATTENUATION 0x3302         // Sets maximum attenuation during soft mute (dB). Set to 0 to disable soft mute. Default is 8 dB.
#define AM_SOFT_MUTE_SNR_THRESHOLD 0x3303           // Sets SNR threshold to engage soft mute. Default is 8 dB.
#define AM_SOFT_MUTE_RELEASE_RATE 0x3304            // Sets softmute release rate. Smaller values provide slower release, and larger values provide faster release.
#define AM_SOFT_MUTE_ATTACK_RATE 0x3305             // Sets software attack rate. Smaller values provide slower attack, and larger values provide faster attack.
#define AM_SEEK_BAND_BOTTOM 0x3400                  // Sets the bottom of the AM band for seek. Default is 520.
#define AM_SEEK_BAND_TOP 0x3401                     // Sets the top of the AM band for seek. Default is 1710.
#define AM_SEEK_FREQ_SPACING 0x3402                 // Selects frequency spacing for AM seek. Default is 10 kHz spacing.
#define AM_SEEK_SNR_THRESHOLD 0x3403                // Sets the SNR threshold for a valid AM Seek/Tune.
#define AM_SEEK_RSSI_THRESHOLD 0x3404               // Sets the RSSI threshold for a valid AM Seek/Tune.
#define AM_AGC_ATTACK_RATE 0x3702                   // Sets the number of milliseconds the high peak detector must be exceeded before decreasing gain.
#define AM_AGC_RELEASE_RATE 0x3703                  // Sets the number of milliseconds the low peak detector must not be exceeded before increasing the gain.
#define AM_FRONTEND_AGC_CONTROL 0x3705              // Adjusts AM AGC for frontend (external) attenuator and LNA.
#define AM_NB_DETECT_THRESHOLD 0x3900               // Sets the threshold for detecting impulses in dB above the noise floor
#define AM_NB_INTERVAL 0x3901                       // Interval in micro-seconds that original samples are replaced by interpolated clean samples
#define AM_NB_RATE 0x3902                           // Noise blanking rate in 100 Hz units. Default value is 64.
#define AM_NB_IIR_FILTER 0x3903                     // Sets the bandwidth of the noise floor estimator. Default value is 300.
#define AM_NB_DELAY 0x3904                          // Delay in micro-seconds before applying impulse blanking to the original samples

#define RX_VOLUME 0x4000
#define RX_HARD_MUTE 0x4001
#define RX_VOLUME_MASK 0x3f

// SSB properties
// See AN332 REV 0.8 Universal Programming Guide (Amendment for SI4735-D60 SSN and NBFM Patches)

#define GPO_IEN 0x0001                       // AM and SSB - Enable interrupt source
#define SSB_BFO 0x0100                       // Sets the Beat Frequency Offset (BFO) under SSB mode.
#define SSB_MODE 0x0101                      // Sets number of properties of the SSB mode.
#define SSB_RSQ_INTERRUPTS 0x3200            // Configure Interrupts related to RSQ
#define SSB_RSQ_SNR_HI_THRESHOLD 0x3201      // Sets high threshold for SNR interrupt
#define SSB_RSQ_SNR_LO_THRESHOLD 0x3202      // Sets low threshold for SNR interrupt
#define SSB_RSQ_RSSI_HI_THRESHOLD 0x3203     // Sets high threshold for RSSI interrupt
#define SSB_RSQ_RSSI_LO_THRESHOLD 0x3204     // Sets low threshold for RSSI interrupt
#define SSB_SOFT_MUTE_RATE 0x3300            // Sets the attack and decay rates when entering or leaving soft mute
#define SSB_SOFT_MUTE_MAX_ATTENUATION 0x3302 // Sets the maximum attenuation during soft mute (db); 0dB to disable soft mute; defaul 8dB;
#define SSB_SOFT_MUTE_SNR_THRESHOLD 0x3303   // Sets SNR threshould to engage soft mute. Defaul 8dB
#define SSB_RF_AGC_ATTACK_RATE 0x3700        // Sets the number of milliseconds the high RF peak detector must be exceeded before decreasing the gain. Defaul 4.
#define SSB_RF_AGC_RELEASE_RATE 0x3701       // Sets the number of milliseconds the low RF peak detector must be exceeded before increasing the gain. Defaul 24.
#define SSB_IF_AGC_RELEASE_RATE 0x3703       // Sets the number of milliseconds the low IF peak detector must not be exceeded before increasing the gain. Default value is 140 (approximately 40 dB / s).
#define SSB_IF_AGC_ATTACK_RATE 0x3702        // Sets the number of milliseconds the high IF peak detector must be exceeded before decreasing gain. Default value is 4 (approximately 1400 dB / s).

// SSB
#define SSB_RF_IF_AGC_ATTACK_RATE 0x3702  // Sets the number of milliseconds the high IF peak detector must be exceeded before decreasing gain. Defaul 4.
#define SSB_RF_IF_AGC_RELEASE_RATE 0x3703 // Sets the number of milliseconds the low IF peak detector must be exceeded before increasing the gain. Defaul 140.

// See AN332 REV 0.8 UNIVERSAL PROGRAMMING GUIDE; pages 12 and 13
#define LSB_MODE 1 // 01
#define USB_MODE 2 // 10

// Parameters
#define SI473X_RDS_OUTPUT_ONLY 0b00000000      // RDS output only (no audio outputs) Si4749 only
#define SI473X_ANALOG_AUDIO 0b00000101         // Analog Audio Inputs
#define SI473X_DIGITAL_AUDIO1 0b00001011       // Digital audio output (DCLK, LOUT/DFS, ROUT/DIO)
#define SI473X_DIGITAL_AUDIO2 0b10110000       // Digital audio outputs (DCLK, DFS, DIO)
#define SI473X_ANALOG_DIGITAL_AUDIO 0b10110101 // Analog and digital audio outputs (LOUT/ROUT and DCLK, DFS,DIO)

// Other parameters
#define FM_CURRENT_MODE 0
#define AM_CURRENT_MODE 1
#define SSB_CURRENT_MODE 2
#define NBFM_CURRENT_MODE 3

#define SEEK_UP 1
#define SEEK_DOWN 0

#define MAX_DELAY_AFTER_SET_FREQUENCY 30 // In ms - This value helps to improve the precision during of getting frequency value
#define MAX_DELAY_AFTER_POWERUP 10       // In ms - Max delay you have to setup after a power up command.
#define MIN_DELAY_WAIT_SEND_LOOP 300     // In uS (Microsecond) - each loop of waitToSend sould wait this value in microsecond
#define MAX_SEEK_TIME 8000               // defines the maximum seeking time 8s is default.

#define DEFAULT_CURRENT_AVC_AM_MAX_GAIN 36

#define XOSCEN_CRYSTAL 1 // Use crystal oscillator
#define XOSCEN_RCLK 0    // Use external RCLK (crystal oscillator disabled).

void SI47xx_init(void);
void SI47xx_setVolume(unsigned char v);
void SI47xx_setFreq(unsigned int f);

void SI47xx_read(char *buf, char length);

#endif // SI47xx_H