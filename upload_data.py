import os
import shutil
import sys
from pathlib import Path

# 获取项目根目录
project_dir = Path(__file__).parent.absolute()
data_dir = project_dir / "data"
data_dest = project_dir / ".pio" / "build" / "esp32-c3-devkitm-1" / "data"

def ensure_dir(directory):
    """确保目录存在，如果不存在则创建"""
    if not os.path.exists(directory):
        os.makedirs(directory)

def copy_data_files():
    """复制数据文件到构建目录"""
    ensure_dir(data_dest)
    
    # 清空目标目录
    for item in os.listdir(data_dest):
        item_path = os.path.join(data_dest, item)
        if os.path.isfile(item_path):
            os.unlink(item_path)
        elif os.path.isdir(item_path):
            shutil.rmtree(item_path)
    
    # 复制数据文件
    for item in os.listdir(data_dir):
        src = os.path.join(data_dir, item)
        dst = os.path.join(data_dest, item)
        if os.path.isfile(src):
            shutil.copy2(src, dst)
            print(f"复制文件: {src} -> {dst}")
        elif os.path.isdir(src):
            shutil.copytree(src, dst)
            print(f"复制目录: {src} -> {dst}")
    
    print("数据文件已准备就绪，可以上传到SPIFFS")

if __name__ == "__main__":
    copy_data_files()
    
    # 调用platformio上传数据命令
    os.system("platformio run --target uploadfs")
    print("数据上传完成")
