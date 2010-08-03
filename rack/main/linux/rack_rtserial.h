/*
 * These defines and structs are copied from
 *
 * www.xenomai.org
 * /include/rtdm/rtserial.h
 * 
 * to be able to use the rtdm API also under rack/linux
 */

#ifndef __RACK_RTSERIAL_H__
#define __RACK_RTSERIAL_H__

#include <limits.h>

#define RTSER_DEF_BAUD              9600

#define RTSER_NO_PARITY             0x00
#define RTSER_ODD_PARITY            0x01
#define RTSER_EVEN_PARITY           0x03
#define RTSER_DEF_PARITY            RTSER_NO_PARITY

#define RTSER_5_BITS                0x00
#define RTSER_6_BITS                0x01
#define RTSER_7_BITS                0x02
#define RTSER_8_BITS                0x03
#define RTSER_DEF_BITS              RTSER_8_BITS

#define RTSER_1_STOPB               0x00

#define RTSER_1_5_STOPB             0x01
#define RTSER_2_STOPB               0x01
#define RTSER_DEF_STOPB             RTSER_1_STOPB

#define RTSER_NO_HAND               0x00
#define RTSER_RTSCTS_HAND           0x01
#define RTSER_DEF_HAND              RTSER_NO_HAND

#define RTSER_FIFO_DEPTH_1          0x00
#define RTSER_FIFO_DEPTH_4          0x40
#define RTSER_FIFO_DEPTH_8          0x80
#define RTSER_FIFO_DEPTH_14         0xC0
#define RTSER_DEF_FIFO_DEPTH        RTSER_FIFO_DEPTH_1

#define RTSER_TIMEOUT_INFINITE      0
#define RTSER_TIMEOUT_NONE          (-1)
#define RTSER_DEF_TIMEOUT           RTSER_TIMEOUT_INFINITE

#define RTSER_RX_TIMESTAMP_HISTORY  0x01
#define RTSER_DEF_TIMESTAMP_HISTORY 0x00

#define RTSER_EVENT_RXPEND          0x01
#define RTSER_EVENT_ERRPEND         0x02
#define RTSER_EVENT_MODEMHI         0x04
#define RTSER_EVENT_MODEMLO         0x08
#define RTSER_DEF_EVENT_MASK        0x00

#define RTSER_SET_BAUD              0x0001
#define RTSER_SET_PARITY            0x0002
#define RTSER_SET_DATA_BITS         0x0004
#define RTSER_SET_STOP_BITS         0x0008
#define RTSER_SET_HANDSHAKE         0x0010
#define RTSER_SET_FIFO_DEPTH        0x0020
#define RTSER_SET_TIMEOUT_RX        0x0100
#define RTSER_SET_TIMEOUT_TX        0x0200
#define RTSER_SET_TIMEOUT_EVENT     0x0400
#define RTSER_SET_TIMESTAMP_HISTORY 0x0800
#define RTSER_SET_EVENT_MASK        0x1000

#define RTSER_LSR_DATA              0x01
#define RTSER_LSR_OVERRUN_ERR       0x02
#define RTSER_LSR_PARITY_ERR        0x04
#define RTSER_LSR_FRAMING_ERR       0x08
#define RTSER_LSR_BREAK_IND         0x10
#define RTSER_LSR_THR_EMTPY         0x20
#define RTSER_LSR_TRANSM_EMPTY      0x40
#define RTSER_LSR_FIFO_ERR          0x80
#define RTSER_SOFT_OVERRUN_ERR      0x0100

#define RTSER_MSR_DCTS              0x01
#define RTSER_MSR_DDSR              0x02
#define RTSER_MSR_TERI              0x04
#define RTSER_MSR_DDCD              0x08
#define RTSER_MSR_CTS               0x10
#define RTSER_MSR_DSR               0x20
#define RTSER_MSR_RI                0x40
#define RTSER_MSR_DCD               0x80

#define RTSER_MCR_DTR               0x01
#define RTSER_MCR_RTS               0x02
#define RTSER_MCR_OUT1              0x04
#define RTSER_MCR_OUT2              0x08
#define RTSER_MCR_LOOP              0x10

typedef struct rtser_config {
    int     config_mask;        
    int     baud_rate;          
    int     parity;             
    int     data_bits;          
    int     stop_bits;          
    int     handshake;          
    int     fifo_depth;         
    int64_t rx_timeout;         
    int64_t tx_timeout;         
    int64_t event_timeout;      
    int     timestamp_history;  
    int     event_mask;         
} rtser_config_t;

typedef struct rtser_status {
    int     line_status;    
    int     modem_status;   
} rtser_status_t;

typedef struct rtser_event {
    int     events;             
    int     rx_pending;         
    uint64_t last_timestamp;    
    uint64_t rxpend_timestamp;  
} rtser_event_t;

#endif // __RACK_RTSERIAL_H__

