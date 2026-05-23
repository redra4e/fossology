# FOSSology K8s Worker Image
# SPDX-FileCopyrightText: © 2024 FOSSology contributors
# SPDX-License-Identifier: GPL-2.0-only

An SSH-reachable worker container that fits the scheduler's existing
remote-host dispatch model. No scheduler modifications are needed for
basic multi-worker operation — just add workers to `[HOSTS]` config.

## Build

```bash
docker build -f docker/worker/Dockerfile -t fossology/worker:latest .
```

## Usage with fo_scheduler

In `fossology.conf`:

```ini
[HOSTS]
localhost = localhost /usr/local/etc/fossology 4
worker-0  = worker-0.fossology.svc.cluster.local /usr/local/etc/fossology 8
```

With capability-aware routing (requires scheduler changes from our PR):

```ini
[HOSTS]
worker-heavy = heavy.svc.local /usr/local/etc/fossology 8 | nomos monk
worker-light = light.svc.local /usr/local/etc/fossology 4 | copyright ojo
```

## Kubernetes Deployment

The worker needs:
1. **SSH secret** mounted at `/run/secrets/ssh/authorized_keys`
2. **Db.conf** mounted at `/usr/local/etc/fossology/Db.conf`
3. **Shared repository volume** at `/srv/fossology/repository`

Example pod spec:
```yaml
containers:
  - name: worker
    image: fossology/worker:latest
    ports:
      - containerPort: 22
    volumeMounts:
      - name: ssh-keys
        mountPath: /run/secrets/ssh
        readOnly: true
      - name: db-conf
        mountPath: /usr/local/etc/fossology/Db.conf
        subPath: Db.conf
        readOnly: true
      - name: repo
        mountPath: /srv/fossology/repository
volumes:
  - name: ssh-keys
    secret:
      secretName: fossology-worker-ssh
  - name: db-conf
    secret:
      secretName: fossology-db-conf
  - name: repo
    persistentVolumeClaim:
      claimName: fossology-repo
```

## Security

- Root login disabled
- Password authentication disabled
- Only pubkey auth for `fossy` user
- TCP forwarding disabled
- X11 forwarding disabled
- MaxStartups tuned for scheduler burst validation (100:30:200)

## Why no agent-wrapper shim?

The scheduler-side changes in this PR handle the startup validation
properly: proxy-managed hosts are skipped during `agent_test()`, and
the retry logic with backoff prevents premature invalidation. No
client-side sleep hack needed.
