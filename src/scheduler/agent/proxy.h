/*
 SPDX-FileCopyrightText: © 2025 Contributors to the FOSSology project

 SPDX-License-Identifier: GPL-2.0-only
*/
/**
 * \file
 * \brief Centralized proxy layer for multi-node agent routing.
 *
 * When the proxy is enabled, hosts marked as "proxy-managed" will have their
 * agents spawned through a configurable proxy command rather than a direct SSH
 * connection.  This allows a Kubernetes-style deployment where heavyweight
 * agents (nomos, monk) run on dedicated pods/nodes and lightweight agents
 * (copyright, ojo) run on smaller ones — without requiring every agent to be
 * installed on every node.
 *
 * Configuration in fossology.conf:
 * \code{.ini}
 * [PROXY]
 * enabled = true
 * ; The command used to route agent processes to the target host.
 * ; Placeholders: %H = target host address, %A = agent_dir on target.
 * command = /usr/local/bin/fo_proxy_route
 * \endcode
 *
 * Hosts behind the proxy use the `proxy` keyword after the optional tag list:
 * \code{.ini}
 * [HOSTS]
 * ; direct host (unchanged behaviour)
 * localhost = localhost /usr/local/etc/fossology 4
 * ; proxy-managed hosts
 * heavy-worker = heavy.local /usr/local/etc/fossology 8 | nomos monk | proxy
 * light-worker = light.local /usr/local/etc/fossology 4 | copyright ojo | proxy
 * \endcode
 */
#ifndef PROXY_H_INCLUDE
#define PROXY_H_INCLUDE

/* local includes */
#include <scheduler.h>

/* other library includes */
#include <gio/gio.h>
#include <glib.h>

#include <stdint.h>

/* ************************************************************************** */
/* **** Data Types ********************************************************** */
/* ************************************************************************** */

/**
 * Proxy routing mode.
 * Controls how agent processes are spawned on remote hosts.
 */
typedef enum
{
    PROXY_DISABLED = 0,   /**< Direct SSH to hosts (default, backward-compatible) */
    PROXY_ENABLED         /**< Route through centralized proxy command */
} proxy_mode_t;

/**
 * Centralized proxy configuration.
 *
 * When enabled, agents destined for proxy-managed hosts are spawned through
 * @c command instead of the default SSH path.  The proxy command receives the
 * target host address and agent binary path so that it can route the request
 * (via SSH jump-host, kubectl exec, docker exec, etc.).
 */
typedef struct proxy_s
{
    proxy_mode_t mode;      /**< Current proxy mode */
    char*    command;        /**< Proxy command (e.g. "/usr/local/bin/fo_proxy_route") */
    char*    address;        /**< Optional proxy network address */
    uint16_t port;           /**< Optional proxy port */
} proxy_t;

/* ************************************************************************** */
/* **** Constructor Destructor ********************************************** */
/* ************************************************************************** */

proxy_t* proxy_init(void);
void     proxy_destroy(proxy_t* proxy);

/* ************************************************************************** */
/* **** Configuration ****************************************************** */
/* ************************************************************************** */

void     proxy_configure(proxy_t* proxy, fo_conf* config);

/* ************************************************************************** */
/* **** Query ************************************************************** */
/* ************************************************************************** */

gboolean proxy_is_enabled(const proxy_t* proxy);

/* ************************************************************************** */
/* **** Status ************************************************************* */
/* ************************************************************************** */

void proxy_print_status(const proxy_t* proxy, GOutputStream* ostr);

#endif /* PROXY_H_INCLUDE */
