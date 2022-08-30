#!/bin/bash

# rootユーザーが必要
if [ $UID -ne 0 ]; then
  echo "Root privileges are required"
  exit 1;
fi

function setup_link() {
  ip netns exec $1 ip addr add $3 dev $2
  ip netns exec $1 ip link set $2 up
  ip netns exec $1 ethtool -K $2 rx off tx off
}

# 全てのnetnsを削除
ip -all netns delete

# 6つのnetnsを作成
ip netns add host1
ip netns add router1
ip netns add router2
ip netns add router3
ip netns add router4
ip netns add host2

# リンクの作成
ip link add name host1-router1 type veth peer name router1-host1 # host1とrouter1のリンク
ip link add name router1-router2 type veth peer name router2-router1 # router1とrouter2のリンク
ip link add name router2-router3 type veth peer name router3-router2 # router2とrouter3のリンク
ip link add name router3-router4 type veth peer name router4-router3 # router3とrouter4のリンク
ip link add name router1-router4 type veth peer name router4-router1 # router1とrouter4のリンク
ip link add name router3-host2 type veth peer name host2-router3 # router3とhost2のリンク

# リンクの割り当て
ip link set host1-router1 netns host1
ip link set router1-host1 netns router1
ip link set router1-router2 netns router1
ip link set router2-router1 netns router2
ip link set router2-router3 netns router2
ip link set router3-router2 netns router3
ip link set router3-router4 netns router3
ip link set router4-router3 netns router4
ip link set router1-router4 netns router1
ip link set router4-router1 netns router4
ip link set router3-host2 netns router3
ip link set host2-router3 netns host2

# host1のリンクの設定
setup_link host1 host1-router1 192.168.0.2/24
ip netns exec host1 ip route add default via 192.168.0.1

# router1のリンクの設定
ip netns exec router1 ip link set router1-host1 up
ip netns exec router1 ethtool -K router1-host1 rx off tx off
ip netns exec router1 ip link set router1-router2 up
ip netns exec router1 ethtool -K router1-router2 rx off tx off
ip netns exec router1 ip link set router1-router4 up
ip netns exec router1 ethtool -K router1-router4 rx off tx off

# router2のリンクの設定
setup_link router2 router2-router1 192.168.1.2/24
setup_link router2 router2-router3 192.168.2.1/24
ip netns exec router2 ip route add 192.168.0.0/24 via 192.168.1.1
ip netns exec router2 ip route add 192.168.5.0/24 via 192.168.2.2
ip netns exec router2 sysctl -w net.ipv4.ip_forward=1

# router3のリンクの設定
setup_link router3 router3-router2 192.168.2.2/24
setup_link router3 router3-router4 192.168.3.1/24
setup_link router3 router3-host2 192.168.5.1/24
ip netns exec router3 ip route add 192.168.0.0/24 via 192.168.2.1
ip netns exec router3 sysctl -w net.ipv4.ip_forward=1

# router4のリンクの設定
setup_link router4 router4-router3 192.168.3.2/24
setup_link router4 router4-router1 192.168.4.1/24
ip netns exec router4 ip route add 192.168.0.0/24 via 192.168.4.2
ip netns exec router4 ip route add 192.168.5.0/24 via 192.168.3.1
ip netns exec router4 sysctl -w net.ipv4.ip_forward=1

# host2のリンクの設定
setup_link host2 host2-router3 192.168.5.2/24
ip netns exec host2 ip route add default via 192.168.5.1


