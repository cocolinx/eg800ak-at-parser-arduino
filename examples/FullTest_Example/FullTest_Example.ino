//
//    FILE: FullTest_Example.ino
//  AUTHOR: CocoLinx
// PURPOSE: test all functions in library
//     URL: https://github.com/cocolinx/eg800ak-at-parser-arduino

#include <CocoLinx_EG800AK_v1.h>

#define TEST_LTE_PLMN_SELECT 	CocoLinx_EG800AK::PLMN_SKT

#define TEST_INTERVAL_SECONDS 180

CocoLinx_EG800AK test;

bool sample_modem_info()
{
  int32_t ret;

  Serial.println("*** sample_modem_info() ***");	

  Serial.print("read manufacturer...");
  char manufacturer[32];
  ret = test.get_cgmi(manufacturer, 32);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("manufacturer: ");
    Serial.println(manufacturer);
  }

  Serial.print("read object_id...");
  char object_id[32];
  ret = test.get_cgmm(object_id, 32);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("object_id: ");
    Serial.println(object_id);
  }

  Serial.print("read revision...");
  char revision[32];
  ret = test.get_cgmr(revision, 32);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("revision: ");
    Serial.println(revision);
  }

  Serial.print("read imei...");
  char imei[32];
  ret = test.get_imei(imei, 32);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("imei: ");
    Serial.println(imei);
  }

  Serial.println("sample_modem_info done!");
  return true;
}

bool sample_lte_connection()
{
  int32_t ret;

  Serial.println("*** sample_lte_connection() ***");	
  Serial.print("set cops...");
  ret = test.set_cops(TEST_LTE_PLMN_SELECT);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else
  {
    Serial.println("okay");
  }

  delay(1000);

  Serial.print("set cfun...");
  ret = test.set_cfun(1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else
  {
    Serial.println("okay");
  }

  delay(1000);

  Serial.print("set cereg...");
  ret = test.set_cereg(1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else
  {
    Serial.println("okay");
  }

  delay(1000);

  Serial.print("read cfun...");
  int32_t fun;
  ret = test.get_cfun(&fun);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("cfun: ");
    Serial.println(fun);
  }

  delay(1000);

  uint32_t start = millis();
  bool connected = false;
  while(millis() - start < 10000)
  {
    Serial.print("read cereg...");
    int32_t stat;
    ret = test.get_cereg(&stat);
    if(ret < 0)
    {
      Serial.print("error: ");
      Serial.println(ret);
      return false;
    }
    else 
    {
      Serial.println("okay");
      Serial.print("get_cereg: ");
      Serial.println(stat);
      if(stat == 5) break;
    }
    delay(1000);  
  }

  Serial.print("read oper...");
  int32_t oper;
  ret = test.get_cops(&oper);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("oper: ");
    Serial.println(oper);
  }

  Serial.print("read rsrp, rsrq...");
  int32_t rsrp, rsrq;
  ret = test.get_cesq(&rsrq, &rsrp);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("rsrp_dbm: ");
    Serial.println(rsrp);
    Serial.print("rsrq_dbm: ");
    Serial.println(rsrq);
  }

  Serial.print("read imsi...");
  char imsi[32];
  ret = test.get_imsi(imsi, 32);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("imsi: ");
    Serial.println(imsi);
  }

  Serial.print("read iccid...");
  char iccid[32];
  ret = test.get_iccid(iccid, 32);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay");
    Serial.print("iccid: ");
    Serial.println(iccid);
  }

  Serial.println("sample_lte_connection done!");
  return true;
}

bool sample_date_time()
{
  int32_t ret;

  Serial.println("*** sample_date_time() ***");	

  Serial.print("set rtc local time...");
  ret = test.set_rtc_auto_local_time();
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay");

  delay(1000);

  Serial.print("read time...");
  char time[32];
  ret = test.get_cclk(time, 32);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else
  {
    int yy, MM, dd, hh, mm, ss;
    Serial.println("okay");
    sscanf(time, "%d/%d/%d,%d:%d:%d", &yy, &MM, &dd, &hh, &mm, &ss);

    Serial.print("year: ");
    Serial.println(2000 + yy);
    Serial.print("month: ");
    Serial.println(MM);
    Serial.print("date: ");
    Serial.println(dd);
    Serial.print("time: ");
    Serial.print(hh);
    Serial.print(":");
    Serial.print(mm);
    Serial.print(":");
    Serial.println(ss);
  }

  Serial.print("read network local time...");
  char networkTime[32];
  ret = test.get_cclk(networkTime, 32);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else
  {
    int yy, MM, dd, hh, mm, ss;
    Serial.println("okay");
    sscanf(networkTime, "%d/%d/%d,%d:%d:%d", &yy, &MM, &dd, &hh, &mm, &ss);

    Serial.print("year: ");
    Serial.println(2000 + yy);
    Serial.print("month: ");
    Serial.println(MM);
    Serial.print("date: ");
    Serial.println(dd);
    Serial.print("time: ");
    Serial.print(hh);
    Serial.print(":");
    Serial.print(mm);
    Serial.print(":");
    Serial.println(ss);
  }

  Serial.println("sample_date_time done!");
  return true;
}

bool sample_udp()
{
  int32_t ret;
  bool is_pdp_activated = false;

  Serial.println("*** sample_udp() ***");	

  Serial.print("get pdp state...");
  int32_t context_state, context_type;
  ret = test.cx_get_pdp_state(1, &context_state, &context_type);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else if(ret == CocoLinx_EG800AK::ACK_NO_PDP)
  {
    Serial.println("okay");
    Serial.println("no pdp activated");
  }
  else
  {
    Serial.println("okay");
    Serial.print("context state: ");
    Serial.println(context_state);
    Serial.print("context type: ");
    Serial.println(context_type);
    is_pdp_activated = true;
  } 

  if(!is_pdp_activated)
  {
    Serial.print("pdp activate...");
    ret = test.cx_pdp_activate(1);
    if(ret < 0)
    {
      Serial.print("error: ");
      Serial.println(ret);
      return false;
    }
    else Serial.println("okay");
  }

  Serial.print("open udp socket...");
  ret = test.cx_socket(1, 0, "UDP", "43.200.166.133", 7777);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay");
  
  delay(1000);

  Serial.print("udp send...");
  char udpTx[] = "hello udp cocolinx~~";
  ret = test.cx_send(0, udpTx, strlen(udpTx));
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay");

  delay(1500);

  Serial.print("upd recv data...");
  char udpRx[32];
  int32_t rxcnt = test.cx_recv(0, udpRx, sizeof(udpRx) - 1);
  if(rxcnt < 0)
  {
    Serial.print("error: ");
    Serial.println(rxcnt);
    return false;
  }
  else
  {
    Serial.println("okay");
    Serial.print("recv> ");
    udpRx[rxcnt] = '\0';
    Serial.println(udpRx);
  }

  Serial.print("udp close...");
  ret = test.cx_close(1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay...");

  Serial.println("sample_udp done!");
  return true;
}

bool sample_tcp()
{
  int32_t ret;
  bool is_pdp_activated = false;

  Serial.println("*** sample_tcp() ***");	

  Serial.print("get pdp state...");
  int32_t context_state, context_type;
  ret = test.cx_get_pdp_state(1, &context_state, &context_type);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else if(ret == CocoLinx_EG800AK::ACK_NO_PDP)
  {
    Serial.println("okay");
    Serial.println("no pdp activated");
  }
  else
  {
    Serial.println("okay");
    Serial.print("context state: ");
    Serial.println(context_state);
    Serial.print("context type: ");
    Serial.println(context_type);
    is_pdp_activated = true;
  } 

  if(!is_pdp_activated)
  {
    Serial.print("pdp activate...");
    ret = test.cx_pdp_activate(1);
    if(ret < 0)
    {
      Serial.print("error: ");
      Serial.println(ret);
      return false;
    }
    else Serial.println("okay");
  }

  Serial.print("open tcp socket...");
  ret = test.cx_socket(1, 0, "TCP", "43.200.166.133", 7777);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay");
  
  delay(1000);

  Serial.print("tcp send...");
  char tcpTx[] = "hello tcp cocolinx~~";
  ret = test.cx_send(0, tcpTx, strlen(tcpTx));
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay");

  delay(1500);

  Serial.print("tcp recv data...");
  char tcpRx[32];
  int32_t rxcnt = test.cx_recv(0, tcpRx, sizeof(tcpRx) - 1);
  if(rxcnt < 0)
  {
    Serial.print("error: ");
    Serial.println(rxcnt);
    return false;
  }
  else
  {
    Serial.println("okay");
    Serial.print("recv> ");
    tcpRx[rxcnt] = '\0';
    Serial.println(tcpRx);
  }

  Serial.print("tcp close...");
  ret = test.cx_close(1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay...");

  Serial.println("sample_tcp done!");
  return true;
}

bool sample_mqtt()
{
  int32_t ret;

  Serial.println("*** sample_mqtt() ***");	

  Serial.print("set mqtt version cfg...");
  ret = test.cx_mqtt_cfg_version(0, 3);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay");

  Serial.print("set mqtt recvmode cfg...");
  ret = test.cx_mqtt_cfg_recvmode(0, 1, 1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay");

  Serial.print("mqtt server open...");
  ret = test.cx_mqtt_open(0, "broker.emqx.io", 1883);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay...");

  delay(2000);

  Serial.print("mqtt connect...");
  ret = test.cx_mqtt_conn(0, "cocolinx", NULL, NULL);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay...");

  delay(2000);

  Serial.print("mqtt topic \"cocolinx/test\" subscribe...");
  ret = test.cx_mqtt_sub(0, 1, "cocolinx/test", 1);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay...");

  delay(2000);

  Serial.print("mqtt topic \"cocolinx/test\" publish...");
  char mqttTx[] = "hello mqtt cocolinx~~";
  ret = test.cx_mqtt_pub(0, 1, 1, 0, "cocolinx/test", mqttTx, strlen(mqttTx));
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay...");

  delay(2000);

  Serial.print("mqtt recv store status check...");
  int32_t store_status[5];
  ret = test.cx_mqtt_recv_info(NULL, store_status, 5);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else 
  {
    Serial.println("okay...");
    for(int i=0; i<5; i++)
    {
      Serial.print("store status[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(store_status[i]);
      Serial.print(" ");
    } 
    Serial.print("\n");
  }

  delay(1000);

  Serial.print("mqtt recv msg from buffer...");
  char mqttrecv[32];
  int32_t rxcnt = test.cx_mqtt_recv(0, 0, NULL, 0, mqttrecv, sizeof(mqttrecv));
  if(rxcnt < 0)
  {
    Serial.print("error: ");
    Serial.println(rxcnt);
    return false;
  }
  else 
  {
    Serial.println("okay...");
    Serial.print("recv> ");
    mqttrecv[rxcnt] = '\0';
    Serial.println(mqttrecv);
  }

  delay(1000);

  Serial.print("mqtt disconnect...");
  ret = test.cx_mqtt_disconn(0);
  if(ret < 0)
  {
    Serial.print("error: ");
    Serial.println(ret);
    return false;
  }
  else Serial.println("okay...");

  Serial.println("sample_mqtt done!");
  return true;
}

void setup() {
	int32_t ret;
	bool success;
	// serial monitor
	Serial.begin(115200);
	while (!Serial) ; //
	Serial.println();
	Serial.println("====== setup() start ======");

  Serial.print("cocolinx begin...");
  ret = test.begin();
  if(!ret)
  {
    Serial.println("error");
		Serial.println("halt forever...");
		while (1);
  } 
  else Serial.println("okay");
}

void loop() {
	static uint32_t millisPrev = -(1000 * 60 * 5); // start first test on first loop
	static uint32_t testCount = 0;

	int32_t ret;
	uint32_t testIntervalMs = (1000 * TEST_INTERVAL_SECONDS);

  if(testIntervalMs < 30000) testIntervalMs = 30000;

  if((millis() - millisPrev) >= testIntervalMs)
  {
    testCount++;

		Serial.println();
		Serial.print("====== test start [");
		Serial.print(testCount);
		Serial.println("] ======");

    sample_modem_info();
    sample_lte_connection();
    sample_date_time();
    sample_udp();
    sample_tcp();
    sample_mqtt();

    Serial.println("===== test done =====");
    millisPrev = millis();
  }

}
