#!/bin/zsh
function die {
	echo "$1" >&2
	exit 1
}

set -x
set -e


[[ $# -eq 3 ]] || die "Usage: $0 [dataset-folder] [tmp-folder] [log-folder]"
typeset -rx kDatasetFolder=$(readlink -f "$1")
typeset -rx kTempFolder=$(readlink -f "$2")
typeset -rx kLogFolder=$(readlink -f "$3")
typeset -rx kOldPwd=$(pwd) 

[[ -n "$CC" ]] || export CC=$(which gcc)
[[ -n "$CXX" ]] || export CXX=$(which g++)


mkdir -p "$kDatasetFolder/ready"
mkdir -p "$kTempFolder"
mkdir -p "$kLogFolder"

local -r kInFileNames=(
english.1
wikipedia
xml
commoncrawl 
fibonacci
gutenberg 
proteins
english
dna
)

local -r kDownloadFiles=(
dna est.fa.xz
wikipedia all_vital.txt.xz
xml pc/dblp.xml.xz
english pc/english.xz
commoncrawl commoncrawl_10240.ascii.xz
fibonacci pc-artificial/fib46.xz
gutenberg gutenberg-201209.24090588160.xz
proteins pc/proteins.xz
)

oldpwd=$(pwd)
cd "$kDatasetFolder"
needDownload=0
for filename in $kInFileNames; do
	if [[ ! -r "$kDatasetFolder/ready/$filename" ]]; then
		needDownload=1
		break
	fi
done
if [[ "$needDownload" -eq 1 ]]; then
	((downloadCounter=-1))
	while
		((downloadCounter+=2))
		[[ $downloadCounter -gt $#kDownloadFiles ]] && break
		[[ -r "$kDatasetFolder/"$kDownloadFiles[$downloadCounter] ]] && continue
		downloadfile=$kDownloadFiles[$downloadCounter+1]
		wget --continue "http://dolomit.cs.tu-dortmund.de/tudocomp/$downloadfile"
		unxz -k $(basename $downloadfile)
	do :; done
	truncate -s 1G english
	
	[[ ! -e ready/english ]] && ln -sv "$kDatasetFolder/english" ready/english
	[[ ! -e ready/english.1 ]] && dd if=english of=ready/english.1 bs=1M count=1
	[[ ! -e ready/dna ]] && ln -sv "$kDatasetFolder/est.fa" ready/dna
	[[ ! -e ready/wikipedia ]] && ln -sv "$kDatasetFolder/all_vital.txt" ready/wikipedia
	[[ ! -e ready/gutenberg ]] && dd if=gutenberg-201209.24090588160 of=ready/gutenberg bs=1000000000 count=1
	[[ ! -e ready/commoncrawl ]] && ln -sv "$kDatasetFolder/commoncrawl_10240.ascii" ready/commoncrawl
	[[ ! -e ready/fibonacci ]] && ln -sv "$kDatasetFolder/fib46" ready/fibonacci
	[[ ! -e ready/xml ]] && ln -sv "$kDatasetFolder/dblp.xml" ready/xml 
	# ./tdc -g 'fib(46)' --usestdout >! fib
fi
cd "$oldpwd"


[[ ! -d hashbench ]] && git clone https://github.com/koeppl/hashbench 
cd hashbench
git submodule init
git submodule update
mkdir -p build
cd build
cmake -DMALLOC_DISABLED=0 ..
make randomcopy 
../scripts/eval_randomcopy.sh | tee randomspace.txt
cmake -DMALLOC_DISABLED=1 ..
make randomcopy 
../scripts/eval_randomcopy.sh | tee randomtime.txt

cd "$kOldPwd"
[[ ! -d tudocomp ]] && git clone --branch lz78 https://github.com/tudocomp/tudocompcd tudocomp 
git submodule init
git submodule update
mkdir -p build
./etc/78scripts/evaluate.sh "$kDatasetFolder/ready" "$kTempFolder" 0 | tee $kLogFolder/tudocomp_memory.log
./etc/78scripts/evaluate.sh "$kDatasetFolder/ready" "$kTempFolder" 1 | tee $kLogFolder/tudocomp_time.log
./etc/78scripts/unix_compress.sh "$kDatasetFolder/ready" "$kTempFolder" | tee $kLogFolder/unixcompress.log


cd "$kOldPwd"
[[ ! -d Low-LZ78 ]] && git clone https://github.com/koeppl/Low-LZ78
cd Low-LZ78 
git submodule init
git submodule update
cd build
./evaluate.sh "$kDatasetFolder/ready" "$kTempFolder" | tee $kLogFolder/low.log

