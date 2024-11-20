import os
import time
import paramiko
import hashlib
import datetime

def is_subdirectory(path:str, parents:list)->bool:
    for parent in parents:
        common_prefix = os.path.commonprefix([path, parent])
        if common_prefix == parent and len(path) >= len(parent):
             return True
    return False

def list_files(startpath:str, filter:list)->list:
    filter_prefixed_paths = [os.path.join(startpath, path) for path in filter]
    ret_list=[]
    for root, dirs, files in os.walk(startpath):
        if is_subdirectory(root, filter_prefixed_paths):
            continue
        for file in files:
            ret_list.append(os.path.join(root, file))
    return ret_list

def calc_md5(file_path):
    """
    计算文件的MD5码
    :param file_path: 文件路径
    :return: 文件的MD5码
    """
    md5 = hashlib.md5()
    with open(file_path, 'rb') as f:
        while True:
            data = f.read(1024)
            if not data:
                break
            md5.update(data)
    return md5.hexdigest()

def path_to_linux(path:str)->str:
    return os.path.normpath(path).replace('\\', '/')
    
def path_to_windows(path:str)->str:
    return os.path.normpath(path).replace('/', '\\')

def path_join_linux(path:str, file:str)->str:
    return path_to_linux(os.path.join(path, file))

def path_join_windows(path:str, file:str)->str:
    return path_to_windows(os.path.join(path, file))

def check_and_copy_files(local_path, remote_path, ssh_host, ssh_port, ssh_username, ssh_password):
    file_list=[]
    ssh_file_md5={}
    #创建实例
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    # 连接远程服务器并进行认证
    client.connect(hostname=ssh_host, username=ssh_username, password=ssh_password,port=ssh_port)
    sftp = client.open_sftp()
    while True:
        #判断有没有新加文件
        new_list = list_files(local_path, ['.git','.vscode'])
        set_c = set(new_list) - set(file_list)
        for file_name in set_c:
            remote_file = path_join_linux(remote_path,file_name.replace(local_path,'', 1))
            remote_file_path = os.path.dirname(remote_file)
            try:
                sftp.stat(remote_file_path)
            except Exception as err:
                stdin, stdout, stderr = client.exec_command(f'mkdir -p "{remote_file_path}"')
                print(f"{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')} mkdir[{remote_file_path}]")
            try:
                sftp.stat(remote_file)
                stdin, stdout, stderr = client.exec_command(f'md5sum {remote_file}')
                ssh_file_md5[file_name]=stdout.read().decode().split()[0]
            except Exception as err:
                print(f"{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')} scp [{file_name}]->[{remote_file}]")
                sftp.put(file_name, remote_file)
                ssh_file_md5[file_name]=calc_md5(file_name)
                
        file_list = new_list
        for file_name in file_list:
            md5=calc_md5(file_name)
            if md5 != ssh_file_md5[file_name]:
                remote_file = path_join_linux(remote_path,file_name.replace(local_path,'', 1))
                print(f"{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')} diff [{file_name}]->[{remote_file}]")
                ssh_file_md5[file_name]= md5
                sftp.put(file_name, remote_file)
                
        print(f"\r{time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())}  ", end='', flush=True)  
        time.sleep(1.0)
    sftp.close()
    client.close() 
 
if __name__ == '__main__':
    local_path='D:\\git\\linux_0\\hello_world_bak\\'
    remote_path='/home/spdk/examples/blob/hello_world_bak/'
    ssh_host = "10.3.71.91" 
    ssh_host = "192.168.118.149" 
    ssh_host = '485069iu73.wicp.vip'
    ssh_port = 22 
    ssh_port = 38442
    ssh_username = "root" 
    ssh_password = "lyj123"
    check_and_copy_files(local_path, remote_path, ssh_host, ssh_port, ssh_username, ssh_password)




