/* datatypes.h 
 * Aidan Fernandes
 * aferna6
 * aferna6-cell
 * ECE 2230 Fall 2025
 * MP3
 *
 * Purpose: Data type definitions for the list ADT
 *
 * Assumptions: alert_t structure defines the data stored in lists
 *
 * Bugs: None known
 */

#ifndef DATATYPES_H
#define DATATYPES_H

struct alert_tag {
    int generator_id;   // ID of component generating alert
    int signature_id;   // ID of detection rule
    int revision_id;    // revision number of detection rule
    int dest_ip_addr;   // IP address of destination
    int src_ip_addr;    // IP address of source 
    int dest_port_num;  // port number at destination
    int src_port_num;   // port number at source host
    int timestamp;      // time in seconds alert received
};

/* The list ADT works on alert data of this type */
typedef struct alert_tag alert_t;
typedef alert_t data_t;

#endif

/* vi:set ts=8 sts=4 sw=4 et: */
