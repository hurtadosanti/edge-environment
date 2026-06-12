#!/usr/bin/env python3
import argparse
import csv
import json
import urllib.request
import urllib.parse
from datetime import datetime

def export_metrics_to_csv(vm_host, output_file, hours):
    # Query VictoriaMetrics for all metrics related to our sensors in the last N hours
    # VictoriaMetrics allows exporting PromQL query results as JSON
    print(f"Connecting to VictoriaMetrics at {vm_host}...")
    
    # We query temperature, humidity, pressure, and window status metrics mapping from VM metric names to CSV columns
    metrics = {
        'mqtt_consumer_temperature': 'temperature',
        'mqtt_consumer_humidity': 'humidity',
        'mqtt_consumer_pressure': 'pressure',
        'mqtt_consumer_status': 'window_status'
    }
    data_by_time_device = {}
    
    for metric, field in metrics.items():
        # Construct PromQL query to get data for the metric
        query = f'{metric}[{hours}h]'
        url_encoded_query = urllib.parse.urlencode({'query': query})
        url = f"http://{vm_host}/api/v1/query?{url_encoded_query}"
        
        try:
            req = urllib.request.Request(url)
            with urllib.request.urlopen(req) as response:
                res_data = json.loads(response.read().decode())
                
                if res_data.get('status') != 'success':
                    print(f"Error querying {metric}: {res_data}")
                    continue
                
                results = res_data.get('data', {}).get('result', [])
                for result in results:
                    # Extract tags (like mac/device_id and address)
                    metric_tags = result.get('metric', {})
                    mac = metric_tags.get('mac') or metric_tags.get('address') or metric_tags.get('client_id') or metric_tags.get('host') or 'unknown'
                    
                    # Values list contains [timestamp, value] pairs
                    values = result.get('values', [])
                    for ts, val in values:
                        # Convert timestamp to ISO RFC3339 string format
                        dt = datetime.fromtimestamp(float(ts))
                        time_str = dt.strftime('%Y-%m-%d %H:%M:%S')
                        
                        key = (time_str, mac)
                        if key not in data_by_time_device:
                            data_by_time_device[key] = {
                                'timestamp': time_str,
                                'device_mac': mac,
                                'temperature': None,
                                'humidity': None,
                                'pressure': None,
                                'window_status': None
                            }
                        data_by_time_device[key][field] = float(val)
                        
        except Exception as e:
            print(f"Failed to fetch {metric}: {e}")
            return False

    if not data_by_time_device:
        print("No data found to export.")
        return False

    # Write to CSV
    print(f"Writing data to {output_file}...")
    with open(output_file, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=['timestamp', 'device_mac', 'temperature', 'humidity', 'pressure', 'window_status'])
        writer.writeheader()
        
        # Sort by timestamp
        for key in sorted(data_by_time_device.keys()):
            writer.writerow(data_by_time_device[key])
            
    print(f"Successfully exported {len(data_by_time_device)} rows to {output_file}!")
    return True

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Export BME280 metrics from VictoriaMetrics to a CSV file.")
    parser.add_argument('--host', default='localhost:8428', help="VictoriaMetrics host:port (default: localhost:8428)")
    parser.add_argument('--output', default='sensor_data.csv', help="Output CSV file path (default: sensor_data.csv)")
    parser.add_argument('--hours', type=int, default=24, help="Number of hours of history to export (default: 24)")
    
    args = parser.parse_args()
    export_metrics_to_csv(args.host, args.output, args.hours)
