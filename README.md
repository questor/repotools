Small tool to work with multiple git repos to keep them up to date and check for updates.

some more methods how to check for updates:
https://github.com/nosarthur/gita/blob/master/gita/info.py




question: do we need to look for updates or do we simply fetch head-revision
and let git handle if it needs an update or not. are we interested in the
change-logs of the libraries?

when not an good option would be to do:;
	git clone --depth=1
	git fetch --depth=1
which will always only download the latest revision blobs without history!
another option would be to download only the commit-history and the blobs of the
latest files by using the blobless clone:
	git clone --filter=blob:none URL
which will download the meta-data and will download only the latest versions of the files

infos: https://github.blog/2020-12-21-get-up-to-speed-with-partial-clone-and-shallow-clone/

[
  {
    "repoPath": "../../_github/_framework/folly/",
    "revision": "37c011d897620fed564a8a6d3d15b9c150fc2609"

"mode": "mirror"/"latestwithhistory","latest"
	mirror - complete history and all blobs, needs dst-url
	latestwithhistory - all trees/commits but blobs only from latest
	latest - no history, only blobs from latest version (--depth=1)
  },

would be great to be able to create an overview of the libraries, for example
scan the readme's or try to grab the short description from github
