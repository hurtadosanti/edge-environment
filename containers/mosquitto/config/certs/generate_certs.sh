#!/bin/bash
set -e

# Directory where the script resides
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR"

# Get the broker hostname or IP from command line argument, default to localhost
HOST="${1:-localhost}"
echo "Generating certificates for MQTT broker: $HOST"

echo "Generating CA key and certificate..."
openssl req -new -x509 -days 3650 -extensions v3_ca -keyout ca.key -out ca.crt -nodes -subj "/CN=MyLocalCA"

echo "Generating server key and CSR..."
openssl req -newkey rsa:2048 -nodes -keyout server.key -out server.csr -subj "/CN=$HOST"

echo "Signing server certificate..."
# Determine if HOST is an IP address or DNS name
if [[ "$HOST" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    SAN="IP.1 = $HOST"
else
    SAN="DNS.1 = $HOST"
fi

cat <<EOF > server.ext
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
$SAN
DNS.2 = localhost
IP.2 = 127.0.0.1
EOF

openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 3650 -extfile server.ext

# Clean up CSR and ext file
rm -f server.csr server.ext ca.srl

echo "Formatting CA cert as a C-string header for Zephyr..."
# Produce ca_cert.inc under firmware/src/
mkdir -p ../../../../firmware/src
awk '{print "\"" $0 "\\n\""}' ca.crt > ../../../../firmware/src/ca_cert.inc

echo "Generating password file for Mosquitto..."
# Try to generate SHA-512 crypt hash, fallback to MD5 crypt if -6 is not supported
PASS_HASH=$(openssl passwd -6 -salt yoursalt12345 device_pass 2>/dev/null || openssl passwd -1 -salt yoursalt12345 device_pass)
echo "device_user:$PASS_HASH" > ../passwd

echo "Certificates and passwords generated successfully in $DIR"
echo "CA cert compiled into: firmware/src/ca_cert.inc"
