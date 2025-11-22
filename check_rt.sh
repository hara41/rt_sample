#!/bin/bash

# PREEMPT_RTカーネル確認スクリプト

echo "=========================================="
echo "PREEMPT_RTカーネル確認"
echo "=========================================="
echo ""

# カーネルバージョンを確認
echo "【カーネル情報】"
uname -a
echo ""

# PREEMPT_RTかどうかを判定
if uname -a | grep -q "PREEMPT_RT\|PREEMPT RT"; then
    echo "✓ PREEMPT_RTカーネルが動作しています"
    echo ""
else
    echo "✗ 標準カーネルが動作しています（PREEMPT_RTではありません）"
    echo ""
    echo "PREEMPT_RTカーネルをインストールするには："
    echo "  sudo rpi-update"
    echo "  または"
    echo "  sudo apt-get install linux-image-rt-arm64"
    echo ""
fi

# プリエンプション設定を確認
if [ -f /sys/kernel/realtime ]; then
    RT_VALUE=$(cat /sys/kernel/realtime)
    echo "【リアルタイム設定】"
    echo "  /sys/kernel/realtime = $RT_VALUE"
    if [ "$RT_VALUE" = "1" ]; then
        echo "  ✓ リアルタイムモードが有効です"
    else
        echo "  ✗ リアルタイムモードが無効です"
    fi
    echo ""
fi

# タイマー解像度を確認
echo "【タイマー解像度】"
if [ -f /proc/timer_list ]; then
    RESOLUTION=$(grep "resolution" /proc/timer_list | head -1)
    echo "  $RESOLUTION"
else
    echo "  確認できません"
fi
echo ""

# リアルタイム優先度の上限を確認
echo "【スケジューリング設定】"
echo "  SCHED_FIFO 最大優先度: $(grep "Max realtime priority" /proc/sys/kernel/sched_rt_runtime_us 2>/dev/null || echo "99")"
RT_RUNTIME=$(cat /proc/sys/kernel/sched_rt_runtime_us 2>/dev/null || echo "-1")
RT_PERIOD=$(cat /proc/sys/kernel/sched_rt_period_us 2>/dev/null || echo "1000000")
echo "  RT runtime: ${RT_RUNTIME} us"
echo "  RT period: ${RT_PERIOD} us"
if [ "$RT_RUNTIME" = "-1" ]; then
    echo "  ✓ リアルタイムスケジューリングに制限なし"
else
    PERCENTAGE=$((RT_RUNTIME * 100 / RT_PERIOD))
    echo "  リアルタイムタスクに割り当て可能: ${PERCENTAGE}%"
fi
echo ""

# メモリロック制限を確認
echo "【メモリロック制限】"
MEMLOCK_SOFT=$(ulimit -S -l)
MEMLOCK_HARD=$(ulimit -H -l)
echo "  ソフトリミット: $MEMLOCK_SOFT"
echo "  ハードリミット: $MEMLOCK_HARD"
if [ "$MEMLOCK_SOFT" = "unlimited" ] || [ "$MEMLOCK_SOFT" -gt 65536 ]; then
    echo "  ✓ メモリロックに十分な制限があります"
else
    echo "  ⚠ メモリロック制限が小さい可能性があります"
    echo "    'ulimit -l unlimited' で制限を解除できます"
fi
echo ""

# GPIOデバイスの確認
echo "【GPIOデバイス】"
if [ -e /dev/gpiochip0 ]; then
    echo "  /dev/gpiochip0: 存在します"
    ls -l /dev/gpiochip0
else
    echo "  /dev/gpiochip0: 見つかりません"
fi
echo ""

echo "=========================================="
echo "確認完了"
echo "=========================================="
