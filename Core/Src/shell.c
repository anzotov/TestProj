#include "netif.h"
#include "stm32f767xx.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#define SHELL_MAX_TOKENS 255
#define SHELL_IP4_ADDR_VAL(IP_ADDR) ip4_addr1_val(IP_ADDR), ip4_addr2_val(IP_ADDR), ip4_addr3_val(IP_ADDR), ip4_addr4_val(IP_ADDR)
#define LED_STATE(STATE) (STATE == GPIO_PIN_SET ? "ON" : "OFF")

typedef char* (*shell_fn)(char**, int);
typedef struct shell_fn_map_t
{
  const char* cmd;
  shell_fn func;
} ShellFnMap;

typedef struct led_map_t
{
  const char* name;
  uint16_t pin;
} LedMap;

static LedMap led_map[] = {
  {"LED1", LD1_Pin},
  {"LED2", LD2_Pin},
  {"LED3", LD3_Pin}
};

static char* print(const char *restrict format, ...)
{
  int bufsz = 0;
  char *buffer = NULL;
  va_list args1;
  va_list args2;
  va_start(args1, format);
  va_copy(args2, args1);
  bufsz = 1 + vsnprintf(buffer, bufsz, format, args1);
  buffer = malloc(bufsz);
  vsnprintf(buffer, bufsz, format, args2);
  va_end(args1);
  va_end(args2);
  return buffer;
}

static char* join(const char *restrict delim, char** strings, int strings_sz)
{
  int result_sz = 1;
  int delim_len = strlen(delim);
  for(int i = 0; i < strings_sz; ++i)
  {
    if(strings[i] == NULL)
    {
      continue;
    }
    if(result_sz != 0)
    {
      result_sz += delim_len;
    }
    result_sz += strlen(strings[i]);
  }
  char *result = malloc(result_sz);
  int offset = 0;
  for(int i = 0; i < strings_sz; ++i)
  {
    if(strings[i] == NULL)
    {
      continue;
    }
    if(offset != 0)
    {
      memcpy(result + offset, delim, delim_len);
      offset += delim_len;
    }
    int len = strlen(strings[i]);
    memcpy(result + offset, strings[i], len);
    offset += len;
    free(strings[i]);
    strings[i] = NULL;
  }
  result[offset] = 0;
  return result;
}

static void tokenize(char* input, char** tokens, int max_tokens)
{
  tokens[0] = input;
  const char* delim = " \t";
  char* state = NULL;
  char* next_token = strtok_r(input, delim, &state);
  for(int i = 0; i < max_tokens && next_token != NULL; ++i)
  {
    tokens[i] = next_token;
    next_token = strtok_r(NULL, delim, &state);
  }
}

static char* ifconfig(char** tokens, int max_tokens)
{
  struct netif* gnetif = netif_find("st");

  return gnetif == NULL ? print("ifconfig error!")
    : print("IP: %u.%u.%u.%u, MASK: %u.%u.%u.%u, GATEWAY: %u.%u.%u.%u",
                                    SHELL_IP4_ADDR_VAL(gnetif->ip_addr),
                                    SHELL_IP4_ADDR_VAL(gnetif->netmask),
                                    SHELL_IP4_ADDR_VAL(gnetif->gw));
}

static char* control_leds(char** tokens, int max_tokens, GPIO_PinState set_state)
{
  const size_t leds_num = sizeof(led_map)/sizeof(led_map[0]);
  uint8_t leds_changed = {0};
  for(int token_n = 1; token_n < max_tokens && tokens[token_n] != NULL; ++token_n)
  {
    for(int i = 0; i < leds_num; ++i)
    {
      if(strcmp(led_map[i].name, tokens[token_n]) == 0)
      {
        HAL_GPIO_WritePin(GPIOB, led_map[i].pin, set_state);
        leds_changed |= 1 << i;
      }
    }
  }
  if(leds_changed == 0)
  {
    return print("No LEDs specified!");
  }
  const char fmt_good[] = "%s turned %s";
  const char fmt_bad[] = "%s stayed %s !!!";
  char* bufs[sizeof(led_map)/sizeof(led_map[0])] = {NULL};
  for(int i = 0; i < leds_num; ++i)
  {
    if((leds_changed & (1 << i)) != 0)
    {
      GPIO_PinState state = HAL_GPIO_ReadPin(GPIOB, led_map[i].pin);
      bufs[i] = (set_state == state) ? print(fmt_good, led_map[i].name, LED_STATE(state)) : print(fmt_bad, led_map[i].name, LED_STATE(state));
    }
  }
  return join(" | ", bufs, leds_num);
}

static char* start_leds(char** tokens, int max_tokens)
{
  GPIO_PinState set_state = GPIO_PIN_SET;
  return control_leds(tokens, max_tokens, set_state);
}

static char* stop_leds(char** tokens, int max_tokens)
{
  GPIO_PinState set_state = GPIO_PIN_RESET;
  return control_leds(tokens, max_tokens, set_state);
}

char* shell_execute_mut(char* input_mut, int input_size)
{
  char* tokens[SHELL_MAX_TOKENS] = {NULL};
  tokenize(input_mut, tokens, sizeof(tokens)/sizeof(tokens[0]));

  char *output = NULL;
  const ShellFnMap map[] = 
  {
    {"ifconfig", &ifconfig},
    {"./start_leds", &start_leds},
    {"./stop_leds", &stop_leds},
  };
  for(int i = 0; i < sizeof(map)/sizeof(map[0]); ++i)
  {
    if(strcmp(map[i].cmd, tokens[0]) == 0)
    {
      output = map[i].func(tokens, sizeof(tokens)/sizeof(tokens[0]));
      break;
    }
  }
  return output;
}

char* shell_execute(const char* input, int input_size)
{
  char* input_mut = malloc(input_size);
  if(input_mut == NULL)
  {
    return NULL;
  }
  memcpy(input_mut, input, input_size);

  char* output = shell_execute_mut(input_mut, input_size);

  free(input_mut);
  return output;
}