#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

/**
 * ボタン入力検出プログラム
 * GPIO27のボタン入力を検出します
 * 実行: sudo ./button_input
 */

#define BUTTON_PIN 2  // GPIO27 (WiringPi pin 2)
#define BOUNCE_TIME 20000  // ボタンのバウンス時間（マイクロ秒）

int main(void)
{
    int button_state, prev_state = LOW;
    
    printf("ボタン入力検出プログラムを開始します\n");
    
    // WiringPiの初期化
    if (wiringPiSetup() == -1)
    {
        printf("WiringPiの初期化に失敗しました\n");
        return 1;
    }
    
    // GPIO27をINPUTモードに設定
    pinMode(BUTTON_PIN, INPUT);
    
    // 内部プルアップを有効化
    pullUpDnControl(BUTTON_PIN, PUD_UP);
    
    printf("ボタン入力を監視しています。Ctrl+Cで終了します\n");
    printf("GPIO27に接続したボタンを押してください\n\n");
    
    while (1)
    {
        // ボタンの状態を読み込む
        button_state = digitalRead(BUTTON_PIN);
        
        // ボタンが押された（HIGHからLOWへの変化）
        if (button_state == LOW && prev_state == HIGH)
        {
            printf("ボタンが押されました！\n");
            
            // バウンス対策：一定時間待機
            usleep(BOUNCE_TIME);
        }
        
        // ボタンが離された（LOWからHIGHへの変化）
        if (button_state == HIGH && prev_state == LOW)
        {
            printf("ボタンが離されました\n");
            
            // バウンス対策：一定時間待機
            usleep(BOUNCE_TIME);
        }
        
        prev_state = button_state;
        
        // ポーリング間隔
        usleep(10000);  // 10ミリ秒
    }
    
    return 0;
}
