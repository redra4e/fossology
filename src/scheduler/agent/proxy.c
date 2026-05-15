/*
 SPDX-FileCopyrightText: © 2025 Contributors to the FOSSology project

 SPDX-License-Identifier: GPL-2.0-only
*/
/**
 * \file
 * \brief Centralized proxy layer for multi-node agent routing.
 */

/* local includes */
#include <proxy.h>
#include <logging.h>

/* std library includes */
#include <string.h>

/* ************************************************************************** */
/* **** Constructor Destructor ********************************************** */
/* ************************************************************************** */

/**
 * @brief Creates a new proxy configuration with proxy disabled.
 *
 * @return Newly allocated proxy configuration (caller must free with
 *         proxy_destroy())
 */
proxy_t* proxy_init(void)
{
  proxy_t* proxy = g_new0(proxy_t, 1);

  proxy->mode    = PROXY_DISABLED;
  proxy->command = NULL;
  proxy->address = NULL;
  proxy->port    = 0;

  return proxy;
}

/**
 * @brief Frees all memory associated with a proxy configuration.
 *
 * @param proxy  The proxy to destroy (may be NULL)
 */
void proxy_destroy(proxy_t* proxy)
{
  if (proxy == NULL)
    return;

  g_free(proxy->command);
  g_free(proxy->address);

  proxy->command = NULL;
  proxy->address = NULL;
  proxy->mode    = PROXY_DISABLED;
  proxy->port    = 0;

  g_free(proxy);
}

/* ************************************************************************** */
/* **** Configuration ****************************************************** */
/* ************************************************************************** */

/**
 * @brief Populate proxy settings from the [PROXY] section of fossology.conf.
 *
 * Expected keys:
 * - @c enabled  – "true" or "1" to enable; anything else disables
 * - @c command  – path to the proxy routing command
 * - @c address  – optional proxy network address
 * - @c port     – optional proxy port
 *
 * If no [PROXY] group exists the proxy remains disabled.
 *
 * @param proxy   The proxy configuration to populate
 * @param config  The loaded fossology.conf
 */
void proxy_configure(proxy_t* proxy, fo_conf* config)
{
  GError* error = NULL;

  if (proxy == NULL || config == NULL)
    return;

  /* If there is no [PROXY] section the feature is simply off */
  if (!fo_config_has_group(config, "PROXY"))
  {
    proxy->mode = PROXY_DISABLED;
    return;
  }

  /* enabled flag */
  if (fo_config_has_key(config, "PROXY", "enabled"))
  {
    char* val = fo_config_get(config, "PROXY", "enabled", &error);
    if (error)
    {
      WARNING("PROXY: failed to read 'enabled': %s\n", error->message);
      g_clear_error(&error);
      return;
    }
    proxy->mode = (g_ascii_strcasecmp(val, "true") == 0 ||
                   strcmp(val, "1") == 0) ? PROXY_ENABLED
                                         : PROXY_DISABLED;
  }

  if (proxy->mode == PROXY_DISABLED)
    return;

  /* command */
  if (fo_config_has_key(config, "PROXY", "command"))
  {
    g_free(proxy->command);
    proxy->command = g_strdup(
        fo_config_get(config, "PROXY", "command", &error));
    if (error)
    {
      WARNING("PROXY: failed to read 'command': %s\n", error->message);
      g_clear_error(&error);
    }
  }

  /* address */
  if (fo_config_has_key(config, "PROXY", "address"))
  {
    g_free(proxy->address);
    proxy->address = g_strdup(
        fo_config_get(config, "PROXY", "address", &error));
    if (error)
    {
      WARNING("PROXY: failed to read 'address': %s\n", error->message);
      g_clear_error(&error);
    }
  }

  /* port */
  if (fo_config_has_key(config, "PROXY", "port"))
  {
    char* val = fo_config_get(config, "PROXY", "port", &error);
    if (error)
    {
      WARNING("PROXY: failed to read 'port': %s\n", error->message);
      g_clear_error(&error);
    }
    else
    {
      proxy->port = (uint16_t)atoi(val);
    }
  }

  /* Validate: proxy enabled without a usable command is a misconfiguration */
  if (proxy->mode == PROXY_ENABLED)
  {
    if (proxy->command == NULL || proxy->command[0] == '\0')
    {
      WARNING("PROXY: enabled but no 'command' configured — disabling proxy\n");
      proxy->mode = PROXY_DISABLED;
    }
    else if (!g_file_test(proxy->command, G_FILE_TEST_EXISTS))
    {
      WARNING("PROXY: command '%s' not found — disabling proxy\n",
              proxy->command);
      proxy->mode = PROXY_DISABLED;
    }
  }

  V_SCHED("PROXY: mode=%s command=%s address=%s port=%d\n",
      proxy->mode == PROXY_ENABLED ? "enabled" : "disabled",
      proxy->command ? proxy->command : "(none)",
      proxy->address ? proxy->address : "(none)",
      proxy->port);
}

/* ************************************************************************** */
/* **** Query ************************************************************** */
/* ************************************************************************** */

/**
 * @brief Check whether the proxy layer is enabled.
 *
 * @param proxy  The proxy configuration (may be NULL)
 * @return TRUE if proxy routing is active
 */
gboolean proxy_is_enabled(const proxy_t* proxy)
{
  return proxy != NULL && proxy->mode == PROXY_ENABLED;
}

/* ************************************************************************** */
/* **** Status ************************************************************* */
/* ************************************************************************** */

/**
 * @brief Write proxy status information to @p ostr.
 *
 * @param proxy  The proxy configuration
 * @param ostr   Output stream to write to
 */
void proxy_print_status(const proxy_t* proxy, GOutputStream* ostr)
{
  char* buf;

  if (proxy == NULL || proxy->mode == PROXY_DISABLED)
  {
    buf = g_strdup("proxy:disabled\n");
  }
  else
  {
    buf = g_strdup_printf(
        "proxy:enabled command:%s address:%s port:%d\n",
        proxy->command ? proxy->command : "(none)",
        proxy->address ? proxy->address : "(none)",
        proxy->port);
  }

  g_output_stream_write(ostr, buf, strlen(buf), NULL, NULL);
  g_free(buf);
}
