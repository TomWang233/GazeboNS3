import socket
import time
import subprocess

# The port to publish on
PORT = 3030

def start_server():
    # Create a TCP/IP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind the socket to the port
    server_socket.bind(('0.0.0.0', PORT))
    server_socket.listen(1)
    print(f"Server is listening on port {PORT}...")

    # Accept a connection
    client_socket, client_address = server_socket.accept()
    print(f"Connection from {client_address}")

    try:
        # Continuous publishing of the latest task
        # Simulate updating the latest task (replace with your actual data source)
        sim_time = get_sim_time()
        hq_pos = get_hq_pos()
        # Send the latest task to the client
        if sim_time:
            client_socket.sendall(f"{sim_time}{hq_pos}".encode('utf-8'))
            print(f"Sent: {sim_time}")
        # Wait a bit before sending the next update
    except BrokenPipeError:
        print("Client disconnected.")
    finally:
        client_socket.close()
        server_socket.close()

def get_sim_time():
    bashCommand = """
    gz topic -e -t /stats -n 1 | awk '/sim_time/,/}/ { 
        if ($1 == "sec:") sec=$2; 
        if ($1 == "nsec:") nsec=$2; 
    }
    END { printf "%.9f\\n", sec + nsec / 1e9 }'
    """
    output = subprocess.check_output(bashCommand, shell=True)
    return output.decode('utf-8')

def get_hq_pos():
    bashCommand = """
    gz topic -e -t /world/iris_runway/pose/info -n 1 | awk '/name: "HQ"/ {found=1} found && /position/ {getline; x=$2; getline; y=$2; getline; z=$2; gsub(",", "", x); gsub(",", "", y); gsub(",", "", z); print x, y, z; found=0}'
    """

    output = subprocess.check_output(bashCommand, shell=True)
    return output.decode('utf-8')

if __name__ == "__main__":
    while True:
        start_server()
