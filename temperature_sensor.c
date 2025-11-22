#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <string.h>

/**
 * 温度センサー読取りプログラム
 * I2C接続のAM2320温度湿度センサーから値を読み取ります
 * デバイスアドレス: 0x5C
 * I2Cバス: /dev/i2c-1
 * 実行: sudo ./temperature_sensor
 */

#define I2C_DEVICE "/dev/i2c-1"
#define SENSOR_ADDR 0x5C  // AM2320のアドレス

int main(void)
{
    int fd;
    unsigned char data[6];
    unsigned short raw_temp, raw_humid;
    float temperature, humidity;
    
    printf("温度・湿度センサー読取りプログラムを開始します\n");
    
    // I2Cデバイスを開く
    fd = open(I2C_DEVICE, O_RDWR);
    if (fd < 0)
    {
        perror("I2Cデバイスのオープンに失敗しました");
        return 1;
    }
    
    // センサーアドレスを設定
    if (ioctl(fd, I2C_SLAVE, SENSOR_ADDR) < 0)
    {
        perror("センサーアドレスの設定に失敗しました");
        close(fd);
        return 1;
    }
    
    printf("I2Cセンサーに接続しました\n");
    printf("読取値を表示します。Ctrl+Cで終了します\n\n");
    
    while (1)
    {
        // センサーからデータを読み込む（6バイト）
        // バイト0-1: 湿度、バイト2-3: 温度、バイト4-5: チェックサム
        if (read(fd, data, 6) != 6)
        {
            perror("センサーからのデータ読込に失敗しました");
        }
        else
        {
            // 湿度データを抽出（バイト0-1）
            raw_humid = (data[0] << 8) | data[1];
            humidity = raw_humid / 10.0;
            
            // 温度データを抽出（バイト2-3）
            raw_temp = (data[2] << 8) | data[3];
            temperature = raw_temp / 10.0;
            
            printf("温度: %.1f℃, 湿度: %.1f%%\n", temperature, humidity);
        }
        
        // 2秒待機
        sleep(2);
    }
    
    close(fd);
    return 0;
}
