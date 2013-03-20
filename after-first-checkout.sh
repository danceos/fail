#!/usr/bin/env bash
#
# Prepare freshly cloned git repo for use with gerrit

## Configure me (if you like..)
SHORT=formaster
GITURL=$(git config --get remote.origin.url)
SSHURL=$(echo "$GITURL"|sed -r 's%^ssh://([^:]+):[^:]+$%\1%')
PORT=$(echo "$GITURL"|sed -r 's%^.+:([0-9]+)/.*$%\1%')
##

CONFIG=.git/config
HOOKDIR=.git/hooks

cd $(dirname $0)
if [ ! -w "$CONFIG" ]; then
    echo -e "\E[31m:( $CONFIG does not exist or is not writable; is this a writable git repository?\E[37m"
    exit 1
fi

# Setup shortcut for pushing changesets: e.g., git push formaster
if grep -q $SHORT $CONFIG; then
    echo -e "\E[32m:) Shortcut \E[33m$SHORT \E[32malready set."
else
    echo -e "\E[32mSetting up \E[33m$SHORT \E[32mshortcut in \E[33m$CONFIG"
    echo -e  "[remote \"$SHORT\"]\n\turl = $GITURL\n\tpush = HEAD:refs/for/master" >> $CONFIG
fi

# Install commit hook setting gerrit change id
if [ -f $HOOKDIR/commit-msg ]
then
    echo -e "\E[32m:) Customized commit message already installed."
    if grep -q "Change-Id:" $HOOKDIR/commit-msg
    then
        echo -e ":) Commit message seems to support Change-Ids."
    else
        echo -e "\E[31m:( Nothing about \E[33mChange-Id \E[31min \E[33m$HOOKDIR/commit-msg"
    fi
else
    echo -e "\E[32m:) Downloading commit message from $SSHURL.\E[37m"
    if ! scp -p -P $PORT "$SSHURL:hooks/commit-msg" .git/hooks; then
        echo -e "\E[31m:( Could not load \E[33mcommit-msg \E[31m"
        echo -e "\E[37mTry manually: \E[33mscp -p -P $PORT $SSHURL:hooks/commit-msg $CONFIG"
    fi
fi

echo -e "\E[37mdone."
