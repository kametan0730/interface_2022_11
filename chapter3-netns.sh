#!/bin/bash

# rootユーザーが必要
if [ $UID -ne 0 ]; then
  echo "Root privileges are required"
  exit 1;
fi

# 全てのnetnsを削除
ip -all netns delete

# 4つのnetnsを作成
ip netns add host1
ip netns add router1
ip netns add router2
ip netns add host2

# リンクの作成
ip link add name host1-router1 type veth peer name router1-host1 # host1とrouter1のリンク
ip link add name router1-router2 type veth peer name router2-router1 # router1とrouter2のリンク
ip link add name router2-host2 type veth peer name host2-router2 # router2とhost2のリンク

# リンクの割り当て
ip link set host1-router1 netns host1
ip link set router1-host1 netns router1
ip link set router1-router2 netns router1
ip link set router2-router1 netns router2
ip link set router2-host2 netns router2
ip link set host2-router2 netns host2

# host1のリンクの設定
ip netns exec host1 ip addr add 192.168.1.2/24 dev host1-router1
ip netns exec host1 ip link set host1-router1 up
ip netns exec host1 ethtool -K host1-router1 rx off tx off
ip netns exec host1 ip route add default via 192.168.1.1

# router1のリンクの設定
ip netns exec router1 ip link set router1-host1 up
ip netns exec router1 ethtool -K router1-host1 rx off tx off
ip netns exec router1 ip link set router1-router2 up
ip netns exec router1 ethtool -K router1-router2 rx off tx off

# router2のリンクの設定
ip netns exec router2 ip addr add 192.168.0.2/24 dev router2-router1
ip netns exec router2 ip link set router2-router1 up
ip netns exec router2 ethtool -K router2-router1 rx off tx off
ip netns exec router2 ip route add default via 192.168.0.1
ip netns exec router2 ip addr add 192.168.2.1/24 dev router2-host2
ip netns exec router2 ip link set router2-host2 up
ip netns exec router2 ethtool -K router2-host2 rx off tx off
ip netns exec router2 sysctl -w net.ipv4.ip_forward=1

# host2のリンクの設定
ip netns exec host2 ip addr add 192.168.2.2/24 dev host2-router2
ip netns exec host2 ip link set host2-router2 up
ip netns exec host2 ethtool -K host2-router2 rx off tx off
ip netns exec host2 ip route add default via 192.168.2.1


