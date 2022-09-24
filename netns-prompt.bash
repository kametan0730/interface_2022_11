if [ `ip netns identify| wc -m` -gt 1 ]; then
        PS1='\[\e]0;\u@\h: \w\a\]${debian_chroot:+($debian_chroot)}\[\033[01;36m\]\u@\h($(ip netns identify))\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
fi
