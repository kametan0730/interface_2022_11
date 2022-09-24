#!/bin/bash

# rootユーザーが必要
if [ $UID -ne 0 ]; then
  echo "Root privileges are required"
  exit 1;
fi

# 全てのnetnsを削除
ip -all netns delete

# bridgeを作成
ip link add br0 type bridge

# 4つのnetnsを作成
ip netns add host0
ip netns add host1
ip netns add router1
ip netns add router2
ip netns add host2

# リンクの作成
ip link add name host0-br0 type veth peer name br0-host0 # host0とbr0のリンク
ip link add name host1-br0 type veth peer name br0-host1 # host1とbr0のリンク
ip link add name router1-br0 type veth peer name br0-router1 # router1とbr0のリンク
ip link add name router1-router2 type veth peer name router2-router1 # router1とrouter2のリンク
ip link add name router2-host2 type veth peer name host2-router2 # router2とhost2のリンク

# ブリッジする
ip link set dev br0-host0 master br0
ip link set dev br0-host1 master br0
ip link set dev br0-router1 master br0
ip link set br0-host0 up
ip link set br0-host1 up
ip link set br0-router1 up
ip link set br0 up

# リンクの割り当て
ip link set host0-br0 netns host0
ip link set host1-br0 netns host1
ip link set router1-br0 netns router1
ip link set router1-router2 netns router1
ip link set router2-router1 netns router2
ip link set router2-host2 netns router2
ip link set host2-router2 netns host2

# host0のリンクの設定
ip netns exec host0 ip addr add 192.168.1.3/24 dev host0-br0
ip netns exec host0 ip link set host0-br0 up
ip netns exec host0 ethtool -K host0-br0 rx off tx off
ip netns exec host0 ip route add default via 192.168.1.1

# host1のリンクの設定
ip netns exec host1 ip addr add 192.168.1.2/24 dev host1-br0
ip netns exec host1 ip link set host1-br0 up
ip netns exec host1 ethtool -K host1-br0 rx off tx off
ip netns exec host1 ip route add default via 192.168.1.1


# router1のリンクの設定
ip netns exec router1 ip link set router1-br0 up
ip netns exec router1 ethtool -K router1-br0 rx off tx off
ip netns exec router1 ip link set router1-router2 up
ip netns exec router1 ethtool -K router1-router2 rx off tx off

# router2のリンクの設定
ip netns exec router2 ip addr add 192.168.0.2/24 dev router2-router1
ip netns exec router2 ip link set router2-router1 up
ip netns exec router2 ethtool -K router2-router1 rx off tx off
ip netns exec router2 ip route add 192.168.1.0/24 via 192.168.0.1
ip netns exec router2 ip addr add 192.168.2.1/24 dev router2-host2
ip netns exec router2 ip link set router2-host2 up
ip netns exec router2 ethtool -K router2-host2 rx off tx off
ip netns exec router2 sysctl -w net.ipv4.ip_forward=1

# host2のリンクの設定
ip netns exec host2 ip addr add 192.168.2.2/24 dev host2-router2
ip netns exec host2 ip link set host2-router2 up
ip netns exec host2 ethtool -K host2-router2 rx off tx off
ip netns exec host2 ip route add default via 192.168.2.1
