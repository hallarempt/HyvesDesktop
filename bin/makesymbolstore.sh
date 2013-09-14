#!/bin/bash
# hack to create a tarball with the revision number on windows

tar -czf ../build/symbolstore-win-r`svn info .. | grep Revision | cut -b11-`.tgz ../build/symbols
