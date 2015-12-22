#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
//#include <string.h>
#include <time.h>
#include "bandwidth.h"

static netsnmp_session session;

u_int prevUP = 0;
u_int prevDOWN = 0;
time_t prevTime;

bandwidth bandwidthRun() {
  netsnmp_session *ss;
  netsnmp_pdu *pdu;
  netsnmp_pdu *response;

  
  size_t downOID_len = OID_LEN;
  oid downOID[downOID_len];

  size_t upOID_len = OID_LEN;
  oid upOID[upOID_len];

  // clear vars
  memset(downOID, 0, sizeof(oid)*downOID_len);
  memset(upOID, 0, sizeof(oid)*upOID_len);

  int status;

  netsnmp_variable_list *vars;

  /*
   * Open the session
   */
  SOCK_STARTUP;

  /* establish the session */
  ss = snmp_open(&session);

  if (!ss) {
    // TODO better error handling
    snmp_sess_perror("ack", &session);
    SOCK_CLEANUP;
    exit(1);
  }

  /*
   * Create the PDU for the data for our request.
   */
  pdu = snmp_pdu_create(SNMP_MSG_GET);
  if (!snmp_parse_oid(SNMP_WAN_DN_OID, downOID, &downOID_len)) {
    // TODO better error handling
    snmp_perror(SNMP_WAN_DN_OID);
    SOCK_CLEANUP;
    exit(1);
  }
  if (!snmp_parse_oid(SNMP_WAN_UP_OID, upOID, &upOID_len)) {
    // TODO better error handling
    snmp_perror(SNMP_WAN_UP_OID);
    SOCK_CLEANUP;
    exit(1);
  }

  snmp_add_null_var(pdu, downOID, downOID_len);
  snmp_add_null_var(pdu, upOID, upOID_len);

  /*
  * Send the Request out.
  */
  status = snmp_synch_response(ss, pdu, &response);
  time_t thisTime;
  time(&thisTime);

  u_int down = prevDOWN;
  u_int up = prevUP;

 /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    for(vars = response->variables; vars; vars = vars->next_variable) {
      //print_variable(vars->name, vars->name_length, vars);
      // my testing
      if (vars->type == ASN_COUNTER) {
        // TODO remove the need for OID_LEN so using whatever the API used for printing
        if (snmp_oid_compare(upOID, upOID_len, vars->name, vars->name_length)) {
          up = (unsigned int)(*vars->val.integer & 0xffffffff);
        } else if (snmp_oid_compare(downOID, downOID_len, vars->name, vars->name_length)) {
          down = (unsigned int)(*vars->val.integer & 0xffffffff);
        } else {
          // got other data?
          continue;
        }

      } else {
        fprintf(stderr, "Got unexpected response type");
      }
    }
  } else {
    /*
     * FAILURE: print what went wrong!
     */
    if (status == STAT_SUCCESS) {
      fprintf(stderr, "Error in packet\nReason: %s\n", snmp_errstring(response->errstat));
    } else if (status == STAT_TIMEOUT) {
      fprintf(stderr, "Timeout: No response from %s.\n", session.peername);
    } else {
      snmp_sess_perror("bandwidth", ss);
    }
  }

  /*
  * Clean up:
  *  1) free the response.
  *  2) close the session.
  */
  if (response) {
    snmp_free_pdu(response);
  }
  snmp_close(ss);

  SOCK_CLEANUP

  // calculate diff
  bandwidth b;
  double diff = difftime(thisTime, prevTime);
  b.down = (down - prevDOWN) /  diff;
  b.up = (up - prevUP) / diff;

  prevTime = thisTime;
  prevDOWN = down;
  prevUP = up;

  return b;
}

void bandwidthInit() {
  /*
   * Initialize the SNMP library
   */
  init_snmp("bandwidth");
  /*
   * Initialize a "session" that defines who we're going to talk to
   */
  snmp_sess_init( &session );                   /* set up defaults */
  session.peername = (char*)SNMP_HOST; // strdup(SNMP_HOST);

  /* set the SNMP version number */
  session.version = SNMP_VERSION_2c;

  /* set the SNMP community name used for authentication */
  session.community = (u_char*)COMMUNITY;
  session.community_len = strlen(COMMUNITY);
}