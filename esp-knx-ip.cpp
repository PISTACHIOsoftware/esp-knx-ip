/**
 * esp-knx-ip library for KNX/IP communication on an ESP8266
 * Author: Nico Weichbrodt <envy>
 * License: Not yet decided on one...
 */

#include "esp-knx-ip.h"

void ESPKNXIP::__handle_root()
{
  String m = F("<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'>");
#ifdef USE_BOOTSTRAP
  m += F("<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta.2/css/bootstrap.min.css' integrity='sha384-PsH8R72JQ3SOdhVi3uxftmaW6Vc51MKb0q5P2rRUpPvrszuE4W1povHYgTpBfshb' crossorigin='anonymous'>");
#endif
  m += F("</head><body><div class='container-fluid'>");
  m += F("<h2>ESP KNX</h2>");
  if (registered_callbacks > 0)
    m += F("<h4>Callbacks</h4>");

  if (registered_ga_callbacks > 0)
  {
    for (uint8_t i = 0; i < registered_ga_callbacks; ++i)
    {
      m += F("<form action='" __DELETE_PATH "' method='POST'>");
      m += F("<div class='row'><div class='col-auto'><div class='input-group'>");
      m += F("<span class='input-group-addon'>");
      m += String((ga_callback_addrs[i].bytes.high & 0xF8) >> 3);
      m += F("/");
      m += String(ga_callback_addrs[i].bytes.high & 0x07);
      m += F("/");
      m += String(ga_callback_addrs[i].bytes.low);
      m += F("</span>");
      m += F("<span class='input-group-addon'>");
      m += callback_names[ga_callbacks[i]];
      m += F("</span>");
      m += F("<input class='form-control' type='hidden' name='id' value='");
      m += i;
      m += F("' /><span class='input-group-btn'><button type='submit' class='btn btn-danger'>Delete</button></span>");
      m += F("</div></div></div>");
      m += F("</form>");
    }
  }

  if (registered_callbacks > 0)
  {
    m += F("<form action='" __REGISTER_PATH "' method='POST'>");
    m += F("<div class='row'><div class='col-auto'><div class='input-group'>");
    m += F("<input class='form-control' type='number' name='area' min='0' max='31'/>");
    m += F("<span class='input-group-addon'>/</span>");
    m += F("<input class='form-control' type='number' name='line' min='0' max='7'/>");
    m += F("<span class='input-group-addon'>/</span>");
    m += F("<input class='form-control' type='number' name='member' min='0' max='255'/>");
    m += F("<span class='input-group-addon'>-&gt;</span>");
    m += F("<select class='form-control' name='cb'>");
    for (uint8_t i = 0; i < registered_callbacks; ++i)
    {
      m += F("<option value=\"");
      m += i;
      m += F("\">");
      m += callback_names[i];
      m += F("</option>");
    }
    m += F("</select>");
    m += F("<span class='input-group-btn'><button type='submit' class='btn btn-primary'>Set</button></span>");
    m += F("</div></div></div>");
    m += F("</form>");
  }

  m += F("<h4>Configuration</h4>");

  // Physical address
  m += F("<form action='" __PHYS_PATH "' method='POST'>");
  m += F("<div class='row'><div class='col-auto'><div class='input-group'>");
  m += F("<span class='input-group-addon'>Physical address</span>");
  m += F("<input class='form-control' type='number' name='area' min='0' max='15' value='");
  m += String((physaddr.bytes.high & 0xF0) >> 4);
  m += F("'/>");
  m += F("<span class='input-group-addon'>.</span>");
  m += F("<input class='form-control' type='number' name='line' min='0' max='15' value='");
  m += String(physaddr.bytes.high & 0x0F);
  m += F("'/>");
  m += F("<span class='input-group-addon'>.</span>");
  m += F("<input class='form-control' type='number' name='member' min='0' max='255' value='");
  m += String(physaddr.bytes.low);
  m += F("'/>");
  m += F("<span class='input-group-btn'><button type='submit' class='btn btn-primary'>Set</button></span>");
  m += F("</div></div></div>");
  m += F("</form>");

  if (registered_configs > 0)
  {
    for (config_id_t i = 0; i < registered_configs; ++i)
    {
      // Check if this config option has a enable condition and if so check that condition
      if (custom_configs[i].cond != nullptr && !custom_configs[i].cond())
        continue;

      m += F("<form action='" __CONFIG_PATH "' method='POST'>");
      m += F("<div class='row'><div class='col-auto'><div class='input-group'>");
      m += F("<span class='input-group-addon'>");
      m += custom_config_names[i];
      m += F("</span>");

      switch (custom_configs[i].type)
      {
        case CONFIG_TYPE_STRING:
          m += F("<input class='form-control' type='text' name='value' value='");
          m += config_get_string(i);
          m += F("' maxlength='");
          m += custom_configs[i].len - 1; // Subtract \0 byte
          m += F("'/>");
          break;
        case CONFIG_TYPE_INT:
          m += F("<input class='form-control' type='number' name='value' value='");
          m += config_get_int(i);
          m += F("'/>");
          break;
        case CONFIG_TYPE_BOOL:
          m += F("<span class='input-group-addon'>");
          m += F("<input type='checkbox' name='value' ");
          if (config_get_bool(i))
            m += F("checked ");
          m += F("/>");
          m += F("</span>");
          break;
        case CONFIG_TYPE_GA:
          address_t a = config_get_ga(i);
          m += F("<input class='form-control' type='number' name='area' min='0' max='31' value='");
          m += String((a.bytes.high & 0xF8) >> 3);
          m += F("'/>");
          m += F("<span class='input-group-addon'>/</span>");
          m += F("<input class='form-control' type='number' name='line' min='0' max='7' value='");
          m += String(a.bytes.high & 0x07);
          m += F("'/>");
          m += F("<span class='input-group-addon'>/</span>");
          m += F("<input class='form-control' type='number' name='member' min='0' max='255' value='");
          m += String(a.bytes.low);
          m += F("'/>");
          break;
      }
      m += F("<input type='hidden' name='id' value='");
      m += i;
      m += F("'/>");
      m += F("<span class='input-group-btn'><button type='submit' class='btn btn-primary'>Set</button></span>");
      m += F("</div></div></div>");
      m += F("</form>");
    }
  }

  // EEPROM save and restore
  m += F("<div class='row'>");
  m += F("<div class='col-auto'>");
  m += F("<form action='" __EEPROM_PATH "' method='POST'>");
  m += F("<input type='hidden' name='mode' value='1'>");
  m += F("<button type='submit' class='btn btn-success'>Save to EEPROM</button>");
  m += F("</form>");
  m += F("</div>");
  m += F("<div class='col-auto'>");
  m += F("<form action='" __EEPROM_PATH "' method='POST'>");
  m += F("<input type='hidden' name='mode' value='2'>");
  m += F("<button type='submit' class='btn btn-info'>Restore from EEPROM</button>");
  m += F("</form>");
  m += F("</div>");
  m += F("</div>"); // row

  // End of page
  m += F("</div></body></html>");
  server->send(200, F("text/html"), m);
}

void ESPKNXIP::__handle_register()
{
  DEBUG_PRINTLN(F("Register called"));
  if (server->hasArg(F("area")) && server->hasArg(F("line")) && server->hasArg(F("member")) && server->hasArg(F("cb")))
  {
    uint8_t area = server->arg(F("area")).toInt();
    uint8_t line = server->arg(F("line")).toInt();
    uint8_t member = server->arg(F("member")).toInt();
    callback_id_t cb = (callback_id_t)server->arg(F("cb")).toInt();

    DEBUG_PRINT(F("Got args: "));
    DEBUG_PRINT(area);
    DEBUG_PRINT(F("/"));
    DEBUG_PRINT(line);
    DEBUG_PRINT(F("/"));
    DEBUG_PRINT(member);
    DEBUG_PRINT(F("/"));
    DEBUG_PRINT(cb);
    DEBUG_PRINTLN(F(""));

    if (area > 31 || line > 7)
    {
      DEBUG_PRINTLN(F("Area or Line wrong"));
      goto end;
    }

    if (cb >= registered_callbacks)
    {
      DEBUG_PRINTLN(F("Invalid callback id"));
      goto end;
    }

    knx.register_GA_callback(area, line, member, cb);
  }
end:
  server->sendHeader(F("Location"),F(__ROOT_PATH));
  server->send(301);
}

void ESPKNXIP::__handle_delete()
{
  DEBUG_PRINTLN(F("Delete called"));
  if (server->hasArg(F("id")))
  {
    uint8_t id = server->arg(F("id")).toInt();

    DEBUG_PRINT(F("Got args: "));
    DEBUG_PRINT(id);
    DEBUG_PRINTLN(F(""));

    if (id >= registered_ga_callbacks)
    {
      DEBUG_PRINTLN(F("ID wrong"));
      goto end;
    }

    knx.delete_GA_callback(id);
  }
end:
  server->sendHeader(F("Location"),F(__ROOT_PATH));
  server->send(301);
}

void ESPKNXIP::__handle_set()
{
  DEBUG_PRINTLN(F("Set called"));
  if (server->hasArg(F("area")) && server->hasArg(F("line")) && server->hasArg(F("member")))
  {
    uint8_t area = server->arg(F("area")).toInt();
    uint8_t line = server->arg(F("line")).toInt();
    uint8_t member = server->arg(F("member")).toInt();

    DEBUG_PRINT(F("Got args: "));
    DEBUG_PRINT(area);
    DEBUG_PRINT(F("."));
    DEBUG_PRINT(line);
    DEBUG_PRINT(F("."));
    DEBUG_PRINT(member);
    DEBUG_PRINTLN(F(""));

    if (area > 31 || line > 7)
    {
      DEBUG_PRINTLN(F("Area or Line wrong"));
      goto end;
    }

    physaddr.bytes.high = (area << 4) | line;
    physaddr.bytes.low = member;
  }
end:
  server->sendHeader(F("Location"),F(__ROOT_PATH));
  server->send(301);
}

void ESPKNXIP::__handle_config()
{
  DEBUG_PRINTLN(F("Config called"));
  if (server->hasArg(F("id")))
  {
    config_id_t id = server->arg(F("id")).toInt();

    DEBUG_PRINT(F("Got args: "));
    DEBUG_PRINT(id);
    DEBUG_PRINTLN(F(""));

    if (id < 0 || id >= registered_configs)
    {
      DEBUG_PRINTLN(F("ID wrong"));
      goto end;
    }

    switch (custom_configs[id].type)
    {
      case CONFIG_TYPE_STRING:
      {
        String v = server->arg(F("value"));
        if (v.length() >= custom_configs[id].len)
          goto end;
        __config_set_string(id, v);
        break;
      }
      case CONFIG_TYPE_INT:
      {
        __config_set_int(id, server->arg(F("value")).toInt());
        break;
      }
      case CONFIG_TYPE_BOOL:
      {
        __config_set_bool(id, server->arg(F("value")).compareTo(F("on")) == 0);
        break;
      }
      case CONFIG_TYPE_GA:
      {
        uint8_t area = server->arg(F("area")).toInt();
        uint8_t line = server->arg(F("line")).toInt();
        uint8_t member = server->arg(F("member")).toInt();
        if (area > 31 || line > 7)
        {
          DEBUG_PRINTLN(F("Area or Line wrong"));
          goto end;
        }
        address_t tmp;
        tmp.bytes.high = (area << 3) | line;
        tmp.bytes.low = member;
        __config_set_ga(id, tmp);
        break;
      }
    }
  }
end:
  server->sendHeader(F("Location"),F(__ROOT_PATH));
  server->send(301);
}

void ESPKNXIP::__handle_eeprom()
{
  DEBUG_PRINTLN(F("EEPROM called"));
  if (server->hasArg(F("mode")))
  {
    uint8_t mode = server->arg(F("mode")).toInt();

    DEBUG_PRINT(F("Got args: "));
    DEBUG_PRINT(mode);
    DEBUG_PRINTLN(F(""));

    if (mode == 1)
    {
      // save
      save_to_eeprom();
    }
    else if (mode == 2)
    {
      // restore
      restore_from_eeprom();
    }
  }
end:
  server->sendHeader(F("Location"),F(__ROOT_PATH));
  server->send(301);
}

ESPKNXIP::ESPKNXIP() : registered_ga_callbacks(0), registered_callbacks(0), registered_configs(0)
{
  physaddr.value = 0;
  memset(custom_config_data, 0, MAX_CONFIG_SPACE * sizeof(uint8_t));
  memset(custom_configs, 0, MAX_CONFIGS * sizeof(config_t));
}

void ESPKNXIP::load()
{
  EEPROM.begin(EEPROM_SIZE);
  restore_from_eeprom();
}

void ESPKNXIP::start(ESP8266WebServer *srv)
{
  if (srv == nullptr)
  {
    return;
  }
  server = srv;
  __start();
}

void ESPKNXIP::start()
{
  server = new ESP8266WebServer(80);
  __start();
}

void ESPKNXIP::__start()
{
  server->on(ROOT_PREFIX, [this](){
    __handle_root();
  });
  server->on(__ROOT_PATH, [this](){
    __handle_root();
  });
  server->on(__REGISTER_PATH, [this](){
    __handle_register();
  });
  server->on(__DELETE_PATH, [this](){
    __handle_delete();
  });
  server->on(__PHYS_PATH, [this](){
    __handle_set();
  });
  server->on(__EEPROM_PATH, [this](){
    __handle_eeprom();
  });
  server->on(__CONFIG_PATH, [this](){
    __handle_config();
  });
  server->begin();

  udp.beginMulticast(WiFi.localIP(),  MULTICAST_IP, MULTICAST_PORT);
}

void ESPKNXIP::save_to_eeprom()
{
  uint32_t address = 0;
  uint64_t magic = EEPROM_MAGIC;
  EEPROM.put(address, magic);
  address += sizeof(uint64_t);
  EEPROM.put(address++, registered_ga_callbacks);
  for (uint8_t i = 0; i < MAX_GA_CALLBACKS; ++i)
  {
    EEPROM.put(address, ga_callback_addrs[i]);
    address += sizeof(address_t);
  }
  for (uint8_t i = 0; i < MAX_GA_CALLBACKS; ++i)
  {
    EEPROM.put(address, ga_callbacks[i]);
    address += sizeof(callback_id_t);
  }
  EEPROM.put(address, physaddr);
  address += sizeof(address_t);

  EEPROM.put(address, custom_config_data);
  address += sizeof(custom_config_data);

  EEPROM.commit();
  DEBUG_PRINT("Wrote to EEPROM: 0x");
  DEBUG_PRINTLN(address, HEX);
}

void ESPKNXIP::restore_from_eeprom()
{
  uint32_t address = 0;
  uint64_t magic = 0;
  EEPROM.get(address, magic);
  if (magic != EEPROM_MAGIC)
  {
    DEBUG_PRINTLN("No valid magic in EEPROM, aborting restore.");
    DEBUG_PRINT("Expected 0x");
    DEBUG_PRINT((unsigned long)(EEPROM_MAGIC >> 32), HEX);
    DEBUG_PRINT(" 0x");
    DEBUG_PRINT((unsigned long)(EEPROM_MAGIC), HEX);
    DEBUG_PRINT(" got 0x");
    DEBUG_PRINT((unsigned long)(magic >> 32), HEX);
    DEBUG_PRINT(" 0x");
    DEBUG_PRINTLN((unsigned long)magic, HEX);
    return;
  }
  address += sizeof(uint64_t);
  EEPROM.get(address++, registered_ga_callbacks);
  for (uint8_t i = 0; i < MAX_GA_CALLBACKS; ++i)
  {
    EEPROM.get(address, ga_callback_addrs[i]);
    address += sizeof(address_t);
  }
  for (uint8_t i = 0; i < MAX_GA_CALLBACKS; ++i)
  {
    EEPROM.get(address, ga_callbacks[i]);
    address += sizeof(callback_id_t);
  }
  EEPROM.get(address, physaddr);
  address += sizeof(address_t); 

  EEPROM.get(address, custom_config_data);
  address += sizeof(custom_config_data);

  DEBUG_PRINT("Restored from EEPROM: 0x");
  DEBUG_PRINTLN(address, HEX);
}

uint16_t ESPKNXIP::ntohs(uint16_t n)
{
  return (uint16_t)((((uint8_t*)&n)[0] << 8) | (((uint8_t*)&n)[1]));
}

int ESPKNXIP::register_GA_callback(uint8_t area, uint8_t line, uint8_t member, callback_id_t cb)
{
  if (registered_ga_callbacks >= MAX_GA_CALLBACKS)
    return -1;

  int id = registered_ga_callbacks;

  ga_callback_addrs[id].bytes.high = (area << 3) | line;
  ga_callback_addrs[id].bytes.low = member;
  ga_callbacks[id] = cb;
  registered_ga_callbacks++;
  return id;
}

int ESPKNXIP::delete_GA_callback(int id)
{
  if (id < 0 || id >= registered_ga_callbacks)
    return -1;

  uint32_t dest_offset = 0;
  uint32_t src_offset = 0;
  uint32_t len = 0;
  if (id == 0)
  {
    // start of array, so delete first entry
    src_offset = 1;
    // registered_ga_callbacks will be 1 in case of only one entry
    // registered_ga_callbacks will be 2 in case of two entries, etc..
    // so only copy anything, if there is it at least more then one element
    len = (registered_ga_callbacks - 1);
  }
  else if (id == registered_ga_callbacks - 1)
  {
    // last element, don't do anything, simply decrement counter
  }
  else
  {
    // somewhere in the middle
    // need to calc offsets

    // skip all prev elements
    dest_offset = id; // id is equal to how many element are in front of it
    src_offset = dest_offset + 1; // start after the current element
    len = (registered_ga_callbacks - 1 - id);
  }

  if (len > 0)
  {
    memmove(ga_callback_addrs + dest_offset, ga_callback_addrs + src_offset, len * sizeof(address_t));
    memmove(ga_callbacks + dest_offset, ga_callbacks + src_offset, len * sizeof(callback_id_t));
  }

  registered_ga_callbacks--;

  return 0;
}

callback_id_t ESPKNXIP::register_callback(String name, GACallback cb)
{
  if (registered_callbacks >= MAX_CALLBACKS)
    return -1;

  callback_id_t id = registered_callbacks;

  callback_names[id] = name;
  callbacks[id] = cb;
  registered_callbacks++;
  return id;
}

callback_id_t ESPKNXIP::register_callback(const char *name, GACallback cb)
{
  return register_callback(String(name), cb);
}

/**
 * Configuration functions start here
 */
config_id_t ESPKNXIP::config_register_string(String name, uint8_t len, String _default, EnableCondition cond)
{
  if (registered_configs >= MAX_CONFIGS)
    return -1;

  if (_default.length() >= len)
    return -1;

  config_id_t id = registered_configs;
  custom_config_names[id] = name;

  custom_configs[id].type = CONFIG_TYPE_STRING;
  custom_configs[id].len = len;
  custom_configs[id].cond = cond;
  if (id == 0)
    custom_configs[id].offset = 0;
  else
    custom_configs[id].offset = custom_configs[id - 1].offset + custom_configs[id - 1].len;

  __config_set_string(id, _default);

  registered_configs++;

  return id;
}

config_id_t ESPKNXIP::config_register_int(String name, int32_t _default, EnableCondition cond)
{
  if (registered_configs >= MAX_CONFIGS)
    return -1;

  config_id_t id = registered_configs;
  custom_config_names[id] = name;

  custom_configs[id].type = CONFIG_TYPE_INT;
  custom_configs[id].len = sizeof(int32_t);
  custom_configs[id].cond = cond;
  if (id == 0)
    custom_configs[id].offset = 0;
  else
    custom_configs[id].offset = custom_configs[id - 1].offset + custom_configs[id - 1].len;

  __config_set_int(id, _default);

  registered_configs++;

  return id;
}

config_id_t ESPKNXIP::config_register_bool(String name, bool _default, EnableCondition cond)
{
  if (registered_configs >= MAX_CONFIGS)
    return -1;

  config_id_t id = registered_configs;
  custom_config_names[id] = name;

  custom_configs[id].type = CONFIG_TYPE_BOOL;
  custom_configs[id].len = sizeof(uint8_t);
  custom_configs[id].cond = cond;
  if (id == 0)
    custom_configs[id].offset = 0;
  else
    custom_configs[id].offset = custom_configs[id - 1].offset + custom_configs[id - 1].len;

  __config_set_bool(id, _default);

  registered_configs++;

  return id;
}

config_id_t ESPKNXIP::config_register_ga(String name, EnableCondition cond)
{
  if (registered_configs >= MAX_CONFIGS)
    return -1;

  config_id_t id = registered_configs;
  custom_config_names[id] = name;

  custom_configs[id].type = CONFIG_TYPE_GA;
  custom_configs[id].len = sizeof(address_t);
  custom_configs[id].cond = cond;
  if (id == 0)
    custom_configs[id].offset = 0;
  else
    custom_configs[id].offset = custom_configs[id - 1].offset + custom_configs[id - 1].len;

  address_t t;
  t.value = 0;
  __config_set_ga(id, t);

  registered_configs++;

  return id;
}

void ESPKNXIP::config_set_string(config_id_t id, String val)
{
  if (id >= registered_configs)
    return;
  if (custom_configs[id].type != CONFIG_TYPE_STRING)
    return;
  if (val.length() >= custom_configs[id].len)
    return;
  __config_set_string(id, val);
}

void ESPKNXIP::__config_set_string(config_id_t id, String &val)
{
  memcpy(&custom_config_data[custom_configs[id].offset], val.c_str(), val.length()+1);
}

void ESPKNXIP::config_set_int(config_id_t id, int32_t val)
{
  if (id >= registered_configs)
    return;
  if (custom_configs[id].type != CONFIG_TYPE_INT)
    return;
  __config_set_int(id, val);
}

void ESPKNXIP::__config_set_int(config_id_t id, int32_t val)
{
  // This does not work for some reason:
  // Could be due to pointer alignment
  //int32_t *v = (int32_t *)(custom_config_data + custom_configs[id].offset);
  //*v = val;

  custom_config_data[custom_configs[id].offset + 0] = (uint8_t)((val & 0xFF000000) >> 24);
  custom_config_data[custom_configs[id].offset + 1] = (uint8_t)((val & 0x00FF0000) >> 16);
  custom_config_data[custom_configs[id].offset + 2] = (uint8_t)((val & 0x0000FF00) >>  8);
  custom_config_data[custom_configs[id].offset + 3] = (uint8_t)((val & 0x000000FF) >>  0);
}

void ESPKNXIP::config_set_bool(config_id_t id, bool val)
{
  if (id >= registered_configs)
    return;
  if (custom_configs[id].type != CONFIG_TYPE_BOOL)
    return;
  __config_set_bool(id, val);
}

void ESPKNXIP::__config_set_bool(config_id_t id, bool val)
{
  custom_config_data[custom_configs[id].offset] = val ? 1 : 0;
}

void ESPKNXIP::config_set_ga(config_id_t id, address_t val)
{
  if (id >= registered_configs)
    return;
  if (custom_configs[id].type != CONFIG_TYPE_GA)
    return;
  __config_set_ga(id, val);
}

void ESPKNXIP::__config_set_ga(config_id_t id, address_t const &val)
{
  custom_config_data[custom_configs[id].offset + 0] = val.bytes.high;
  custom_config_data[custom_configs[id].offset + 1] = val.bytes.low;
}

String ESPKNXIP::config_get_string(config_id_t id)
{
  if (id >= registered_configs)
    return String("");

  return String((char *)&custom_config_data[custom_configs[id].offset]);
}

int32_t ESPKNXIP::config_get_int(config_id_t id)
{
  if (id >= registered_configs)
    return 0;

  int32_t v = (custom_config_data[custom_configs[id].offset + 0] << 24) +
              (custom_config_data[custom_configs[id].offset + 1] << 16) +
              (custom_config_data[custom_configs[id].offset + 2] <<  8) +
              (custom_config_data[custom_configs[id].offset + 3] <<  0);
  return v;
}

bool ESPKNXIP::config_get_bool(config_id_t id)
{
  if (id >= registered_configs)
    return false;

  return custom_config_data[custom_configs[id].offset] != 0;
}

address_t ESPKNXIP::config_get_ga(config_id_t id)
{
  address_t t;
  if (id >= registered_configs)
  {
    t.value = 0;
    return t;
  }

  t.bytes.high = custom_config_data[custom_configs[id].offset + 0];
  t.bytes.low = custom_config_data[custom_configs[id].offset + 1];

  return t;
}

/**
 * Send functions start here
 */

void ESPKNXIP::send(address_t const &receiver, knx_command_type_t ct, uint8_t data_len, uint8_t *data)
{
  if (receiver.value == 0)
    return;

  uint32_t len = 6 + 2 + 8 + data_len + 1; // knx_pkt + cemi_msg + cemi_service + data + checksum
  DEBUG_PRINT(F("Creating packet with len "));
  DEBUG_PRINTLN(len)
  uint8_t buf[len];
  knx_ip_pkt_t *knx_pkt = (knx_ip_pkt_t *)buf;
  knx_pkt->header_len = 0x06;
  knx_pkt->protocol_version = 0x10;
  knx_pkt->service_type = ntohs(KNX_ST_ROUTING_INDICATION);
  knx_pkt->total_len.len = ntohs(len);
  cemi_msg_t *cemi_msg = (cemi_msg_t *)knx_pkt->pkt_data;
  cemi_msg->message_code = KNX_MT_L_DATA_IND;
  cemi_msg->additional_info_len = 0;
  cemi_service_t *cemi_data = &cemi_msg->data.service_information;
  cemi_data->control_1.bits.confirm = 0;
  cemi_data->control_1.bits.ack = 0;
  cemi_data->control_1.bits.priority = B11;
  cemi_data->control_1.bits.system_broadcast = 0x01;
  cemi_data->control_1.bits.repeat = 0x01;
  cemi_data->control_1.bits.reserved = 0;
  cemi_data->control_1.bits.frame_type = 0x01;
  cemi_data->control_2.bits.extended_frame_format = 0x00;
  cemi_data->control_2.bits.hop_count = 0x06;
  cemi_data->control_2.bits.dest_addr_type = 0x01;
  cemi_data->source = physaddr;
  cemi_data->destination = receiver;
  //cemi_data->destination.bytes.high = (area << 3) | line;
  //cemi_data->destination.bytes.low = member;
  cemi_data->data_len = data_len;
  cemi_data->pci.apci = (ct & 0x0C) >> 2;
  cemi_data->pci.tpci_seq_number = 0x00; // ???
  cemi_data->pci.tpci_comm_type = KNX_COT_UDP; // ???
  memcpy(cemi_data->data, data, data_len);
  cemi_data->data[0] = (cemi_data->data[0] & 0x3F) | ((ct & 0x03) << 6);

  // Calculate checksum, which is just XOR of all bytes
  uint8_t cs = buf[0] ^ buf[1];
  for (uint32_t i = 2; i < len - 1; ++i)
  {
    cs ^= buf[i];
  }
  buf[len - 1] = cs;

  DEBUG_PRINT(F("Sending packet:"));
  for (int i = 0; i < len; ++i)
  {
    DEBUG_PRINT(F(" 0x"));
    DEBUG_PRINT(buf[i], 16);
  }
  DEBUG_PRINTLN(F(""));

  udp.beginPacketMulticast(MULTICAST_IP, MULTICAST_PORT, WiFi.localIP());
  udp.write(buf, len);
  udp.endPacket();
}

void ESPKNXIP::send1Bit(address_t const &receiver, knx_command_type_t ct, uint8_t bit)
{
  uint8_t buf[] = {bit & 0b00000001};
  send(receiver, ct, 1, buf);
}

void ESPKNXIP::send2Bit(address_t const &receiver, knx_command_type_t ct, uint8_t twobit)
{
  uint8_t buf[] = {twobit & 0b00000011};
  send(receiver, ct, 1, buf);
}

void ESPKNXIP::send4Bit(address_t const &receiver, knx_command_type_t ct, uint8_t fourbit)
{
  uint8_t buf[] = {fourbit & 0b00001111};
  send(receiver, ct, 1, buf);
}

void ESPKNXIP::send1ByteInt(address_t const &receiver, knx_command_type_t ct, int8_t val)
{
  uint8_t buf[] = {0x00, (uint8_t)val};
  send(receiver, ct, 2, buf);
}

void ESPKNXIP::send2ByteInt(address_t const &receiver, knx_command_type_t ct, int16_t val)
{
  uint8_t buf[] = {0x00, (uint8_t)(val >> 8), (uint8_t)(val & 0x00FF)};
  send(receiver, ct, 3, buf);
}

void ESPKNXIP::send2ByteFloat(address_t const &receiver, knx_command_type_t ct, float val)
{
  float v = val * 100.0f;
  int e = 0;
  for (; v < -2048.0f; v /= 2)
    ++e;
  for (; v > 2047.0f; v /= 2)
    ++e;
  long m = round(v) & 0x7FF;
  short msb = (short) (e << 3 | m >> 8);
  if (val < 0.0f)
    msb |= 0x80;
  uint8_t buf[] = {0x00, (uint8_t)msb, (uint8_t)m};
  send(receiver, ct, 3, buf);
}

void ESPKNXIP::send3ByteTime(address_t const &receiver, knx_command_type_t ct, uint8_t weekday, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
  weekday <<= 5;
  uint8_t buf[] = {0x00, (((weekday << 5) & 0xE0) + (hours & 0x1F)), minutes & 0x3F, seconds & 0x3F};
  send(receiver, ct, 4, buf);
}

void ESPKNXIP::send3ByteDate(address_t const &receiver, knx_command_type_t ct, uint8_t day, uint8_t month, uint8_t year)
{
  uint8_t buf[] = {0x00, day & 0x1F, month & 0x0F, year};
  send(receiver, ct, 4, buf);
}

void ESPKNXIP::send3ByteColor(address_t const &receiver, knx_command_type_t ct, uint8_t red, uint8_t green, uint8_t blue)
{
  uint8_t buf[] = {0x00, red, green, blue};
  send(receiver, ct, 4, buf);
}

void ESPKNXIP::send4ByteFloat(address_t const &receiver, knx_command_type_t ct, float val)
{
  uint8_t buf[] = {0x00, ((uint8_t *)&val)[3], ((uint8_t *)&val)[2], ((uint8_t *)&val)[1], ((uint8_t *)&val)[0]};
  send(receiver, ct, 5, buf);
}

void ESPKNXIP::loop()
{
  __loop_knx();
  __loop_webserver();
}

void ESPKNXIP::__loop_webserver()
{
  server->handleClient();
}

void ESPKNXIP::__loop_knx()
{
  int read = udp.parsePacket();
  if (!read)
    return;
  DEBUG_PRINTLN(F(""));
  DEBUG_PRINT(F("LEN: "));
  DEBUG_PRINTLN(read);

  uint8_t buf[read];

  udp.read(buf, read);

  DEBUG_PRINT(F("Got packet:"));
  for (int i = 0; i < read; ++i)
  {
    DEBUG_PRINT(F(" 0x"));
    DEBUG_PRINT(buf[i], 16);
  }
  DEBUG_PRINTLN(F(""));

  knx_ip_pkt_t *knx_pkt = (knx_ip_pkt_t *)buf;

  DEBUG_PRINT(F("ST: 0x"));
  DEBUG_PRINTLN(ntohs(knx_pkt->service_type), 16);

  if (knx_pkt->header_len != 0x06 && knx_pkt->protocol_version != 0x10 && knx_pkt->service_type != KNX_ST_ROUTING_INDICATION)
    return;

  cemi_msg_t *cemi_msg = (cemi_msg_t *)knx_pkt->pkt_data;

  DEBUG_PRINT(F("MT: 0x"));
  DEBUG_PRINTLN(cemi_msg->message_code, 16);

  if (cemi_msg->message_code != KNX_MT_L_DATA_IND)
    return;

  DEBUG_PRINT(F("ADDI: 0x"));
  DEBUG_PRINTLN(cemi_msg->additional_info_len, 16);

  cemi_service_t *cemi_data = &cemi_msg->data.service_information;

  if (cemi_msg->additional_info_len > 0)
    cemi_data = (cemi_service_t *)(((uint8_t *)cemi_data) + cemi_msg->additional_info_len);

  DEBUG_PRINT(F("C1: 0x"));
  DEBUG_PRINTLN(cemi_data->control_1.byte, 16);

  DEBUG_PRINT(F("C2: 0x"));
  DEBUG_PRINTLN(cemi_data->control_2.byte, 16);

  DEBUG_PRINT(F("DT: 0x"));
  DEBUG_PRINTLN(cemi_data->control_2.bits.dest_addr_type, 16);

  if (cemi_data->control_2.bits.dest_addr_type != 0x01)
    return;

  DEBUG_PRINT(F("HC: 0x"));
  DEBUG_PRINTLN(cemi_data->control_2.bits.hop_count, 16);

  DEBUG_PRINT(F("EFF: 0x"));
  DEBUG_PRINTLN(cemi_data->control_2.bits.extended_frame_format, 16);

  DEBUG_PRINT(F("Source: 0x"));
  DEBUG_PRINT(cemi_data->source.bytes.high, 16);
  DEBUG_PRINT(F(" 0x"));
  DEBUG_PRINTLN(cemi_data->source.bytes.low, 16);

  DEBUG_PRINT(F("Dest: 0x"));
  DEBUG_PRINT(cemi_data->destination.bytes.high, 16);
  DEBUG_PRINT(F(" 0x"));
  DEBUG_PRINTLN(cemi_data->destination.bytes.low, 16);

  knx_command_type_t ct = (knx_command_type_t)(((cemi_data->data[0] & 0xC0) >> 6) | ((cemi_data->pci.apci & 0x03) << 2));

  DEBUG_PRINT(F("CT: 0x"));
  DEBUG_PRINTLN(ct, 16);

  for (int i = 0; i < cemi_data->data_len; ++i)
  {
    DEBUG_PRINT(F(" 0x"));
    DEBUG_PRINT(cemi_data->data[i], 16);
  }

  DEBUG_PRINTLN(F("=="));

  // Call callbacks
  for (int i = 0; i < registered_ga_callbacks; ++i)
  {
    DEBUG_PRINT(F("Testing: 0x"));
    DEBUG_PRINT(ga_callback_addrs[i].bytes.high, 16);
    DEBUG_PRINT(F(" 0x"));
    DEBUG_PRINTLN(ga_callback_addrs[i].bytes.low, 16);
    if (cemi_data->destination.value == ga_callback_addrs[i].value)
    {
      DEBUG_PRINTLN(F("Found match"));
      uint8_t data[cemi_data->data_len];
      memcpy(data, cemi_data->data, cemi_data->data_len);
      data[0] = data[0] & 0x3F;
      callbacks[ga_callbacks[i]](ct, cemi_data->destination, cemi_data->data_len, data);
      return;
    }
  }

  return;
}

// Global "singleton" object
ESPKNXIP knx;