# FOSSology Worker Container for Kubernetes
# SPDX-FileCopyrightText: © 2024 FOSSology contributors
# SPDX-License-Identifier: GPL-2.0-only
#
# This image provides an SSH-reachable worker pod that the fo_scheduler can
# dispatch agents to using its existing remote-host model. The scheduler
# connects over SSH exactly as it does with bare-metal remote hosts.
#
# Build:
#   docker build -f docker/worker/Dockerfile -t fossology/worker:latest .
#
# The scheduler's [HOSTS] config points at this pod's service DNS name.

ARG FOSSOLOGY_VERSION=4.4.0
FROM fossology/fossology:${FOSSOLOGY_VERSION}

# Install and harden OpenSSH server for scheduler-to-worker dispatch.
RUN apt-get update && \
    apt-get install -y --no-install-recommends openssh-server && \
    rm -rf /var/lib/apt/lists/* && \
    mkdir -p /run/sshd /home/fossy/.ssh && \
    chmod 700 /home/fossy/.ssh && \
    chown fossy:fossy /home/fossy/.ssh && \
    # Harden sshd: pubkey only, no root login, no forwarding
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin no/' /etc/ssh/sshd_config && \
    sed -i 's/PermitRootLogin yes/PermitRootLogin no/' /etc/ssh/sshd_config && \
    sed -i 's/#PubkeyAuthentication yes/PubkeyAuthentication yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication no/' /etc/ssh/sshd_config && \
    sed -i 's/PasswordAuthentication yes/PasswordAuthentication no/' /etc/ssh/sshd_config && \
    sed -i 's/#AllowTcpForwarding yes/AllowTcpForwarding no/' /etc/ssh/sshd_config && \
    sed -i 's/#X11Forwarding yes/X11Forwarding no/' /etc/ssh/sshd_config && \
    sed -i 's/X11Forwarding yes/X11Forwarding no/' /etc/ssh/sshd_config && \
    # Handle burst of scheduler startup tests (~26 agents validated at once)
    sed -i 's/#MaxStartups 10:30:100/MaxStartups 100:30:200/' /etc/ssh/sshd_config && \
    sed -i 's/#MaxSessions 10/MaxSessions 50/' /etc/ssh/sshd_config && \
    sed -i 's/#LogLevel INFO/LogLevel VERBOSE/' /etc/ssh/sshd_config && \
    # Only allow fossy user
    echo "AllowUsers fossy" >> /etc/ssh/sshd_config && \
    # Generate host keys at build time (regenerated at runtime if missing)
    ssh-keygen -A

COPY docker/worker/entrypoint.sh /entrypoint-worker.sh
RUN chmod +x /entrypoint-worker.sh

EXPOSE 22

ENTRYPOINT ["/bin/sh", "/entrypoint-worker.sh"]
