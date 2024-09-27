# coding: utf-8
import sys

def add_host_entry(ip, domain):
    entry = f"{ip} {domain}\n"
    try:
        with open("/etc/hosts", "a") as file:
            file.write(entry)
        print(f"Added entry: {entry}")
    except PermissionError:
        print("Permission denied. Please run the script with sufficient privileges.")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 add_host_entry.py <IP> <DOMAIN>")
        sys.exit(1)

    ip = sys.argv[1]
    domain = sys.argv[2]
    add_host_entry(ip, domain)
