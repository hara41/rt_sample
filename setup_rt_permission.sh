#!/bin/bash

# 一般ユーザーでもリアルタイム優先度を使えるように設定するスクリプト

echo "=========================================="
echo "リアルタイム権限の設定"
echo "=========================================="
echo ""

# 現在のユーザーを取得
CURRENT_USER=${SUDO_USER:-$USER}

echo "ユーザー '${CURRENT_USER}' にリアルタイム権限を付与します"
echo ""

# limits.confに設定を追加
LIMITS_FILE="/etc/security/limits.conf"
BACKUP_FILE="${LIMITS_FILE}.backup.$(date +%Y%m%d_%H%M%S)"

# バックアップを作成
if [ -f "$LIMITS_FILE" ]; then
    echo "1. 既存の設定をバックアップ中..."
    sudo cp "$LIMITS_FILE" "$BACKUP_FILE"
    echo "   バックアップ: $BACKUP_FILE"
    echo ""
fi

# 既存の設定を確認
if grep -q "^${CURRENT_USER}.*rtprio" "$LIMITS_FILE" 2>/dev/null; then
    echo "⚠ 既にリアルタイム優先度の設定が存在します"
    grep "^${CURRENT_USER}.*rtprio" "$LIMITS_FILE"
    echo ""
    read -p "上書きしますか？ (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "キャンセルしました"
        exit 0
    fi
    # 既存の設定を削除
    sudo sed -i "/^${CURRENT_USER}.*rtprio/d" "$LIMITS_FILE"
    sudo sed -i "/^${CURRENT_USER}.*memlock/d" "$LIMITS_FILE"
fi

# 新しい設定を追加
echo "2. リアルタイム権限を設定中..."
echo "${CURRENT_USER}    -    rtprio    99" | sudo tee -a "$LIMITS_FILE" > /dev/null
echo "${CURRENT_USER}    -    memlock   unlimited" | sudo tee -a "$LIMITS_FILE" > /dev/null

echo "   ✓ リアルタイム優先度: 99"
echo "   ✓ メモリロック: unlimited"
echo ""

# 設定を確認
echo "3. 設定内容の確認:"
grep "^${CURRENT_USER}" "$LIMITS_FILE" | grep -E "rtprio|memlock"
echo ""

echo "=========================================="
echo "設定完了"
echo "=========================================="
echo ""
echo "注意: 設定を有効にするには、一度ログアウトして"
echo "      再度ログインする必要があります。"
echo ""
echo "設定確認コマンド（再ログイン後）:"
echo "  ulimit -r     # リアルタイム優先度の上限"
echo "  ulimit -l     # メモリロックの上限"
echo ""
echo "設定が有効になれば、sudo なしで実行できます:"
echo "  ./led_control"
echo "  ./button_input"
echo ""
