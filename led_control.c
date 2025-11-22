#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

/**
 * LED制御プログラム
 * GPIO17のLEDをON/OFFで制御します
 * 実行: sudo ./led_control
 */

#define LED_PIN 0  // GPIO17 (WiringPi pin 0)
#define INTERVAL 500000  // マイクロ秒 (0.5秒)

int main(void)
{
    printf("LED制御プログラムを開始します\n");
    
    // WiringPiの初期化
    if (wiringPiSetup() == -1)
    {
        printf("WiringPiの初期化に失敗しました\n");
        return 1;
    }
    
    // GPIO17をOUTPUTモードに設定
    pinMode(LED_PIN, OUTPUT);
    
    printf("LED制御を開始します。Ctrl+Cで終了します\n");
    printf("GPIO17が点灯・消灯を繰り返します\n");
    
    // LED点灯・消灯ループ（無限ループ）
    while (1)
    {
        // LED点灯
        digitalWrite(LED_PIN, HIGH);
        printf("LED: ON\n");
        usleep(INTERVAL);
        
        // LED消灯
        digitalWrite(LED_PIN, LOW);
        printf("LED: OFF\n");
        usleep(INTERVAL);
    }
    
    return 0;
}
