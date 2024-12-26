# How to develop

## Create virtual environment

```bash
python -m venv .venv
```

## Activate virtual environment

### MAC

```bash
source .venv/bin/activate
```

### Windows

```bash
.venv\Scripts\activate
```

## Install dependencies

### Install West

```bash
pip install west
```

### Install Zephyr requirements

```bash
pip install -r deps/zephyr/scripts/requirements.txt
```

## Build

```bash
west build app -p auto -b heltec_wifi_lora32_v3/esp32s3/procpu
```

## Flash

```bash
west flash
```

## Monitor

```bash
west espressif monitor
```

## Good to know

- When you change the `app/west.yml` file, you need to reconfigure the project.

```bash
west update
```

### Useful alias

```bash
# Alias for zephyr
alias zephyr-venv='source .venv/bin/activate'
alias zephyr-build='west build app -p always -b heltec_wifi_lora32_v3/esp32s3/procpu'
alias zephyr-flash='west flash'
alias zephyr-monitor='west espressif monitor'
```
