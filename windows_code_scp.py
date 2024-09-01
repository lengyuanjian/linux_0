import paramiko
import os
from scp import SCPClient

def create_ssh_client(hostname, port, username, password):
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname, port, username, password)
    return ssh

def get_directory(local_dir, remote_dir, ssh_client):
    scp = SCPClient(ssh_client.get_transport(), socket_timeout=30.0)

    # # Ensure the remote directory exists
    # stdin, stdout, stderr = ssh_client.exec_command(f'mkdir -p {remote_dir}')
    # stdout.channel.recv_exit_status()

    # Recursively copy the directory
    # scp.put(local_dir, remote_path=remote_dir, recursive=True)
    scp.get(remote_path=remote_dir,local_path=local_dir, recursive=True)
    scp.close()

def send_directory(local_dir, remote_dir, ssh_client):
    scp = SCPClient(ssh_client.get_transport(), socket_timeout=30.0)

    # # Ensure the remote directory exists
    # stdin, stdout, stderr = ssh_client.exec_command(f'mkdir -p {remote_dir}')
    # stdout.channel.recv_exit_status()

    # Recursively copy the directory
    scp.put(local_dir, remote_path=remote_dir, recursive=True)
     

    scp.close()

if __name__ == '__main__':
    hostname = "485069iu73.wicp.vip"     # 替换为远程主机的IP或主机名
    port = 38442                         # SSH端口，通常是22
    username = "root"        # 替换为你的用户名
    password = "lyj123"        # 替换为你的密码

    local_directory = "D:\\git\\linux_0\\"  # 替换为本地Windows目录路径，使用双反斜杠
    remote_directory = "/home/spdk/examples/blob/hello_world_bak/"      # 替换为远程目录路径

    ssh_client = create_ssh_client(hostname, port, username, password)

    if not os.path.exists(local_directory):
        os.makedirs(local_directory)
        print(f"Local directory '{local_directory}' created.")

    try:
        get_directory(local_directory, remote_directory, ssh_client)
        print(f"get:{remote_directory} -> {local_directory} successfully!")
    except Exception as e:
        print(f"Failed to copy directory: {e}")
    finally:
        ssh_client.close()
