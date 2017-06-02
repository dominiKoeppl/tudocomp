#!/usr/bin/python3

import re
import sys
import itertools
import argparse
import pprint
import hashlib
import collections
import os
import time

from operator import itemgetter, attrgetter
from textwrap import dedent, indent
from collections import OrderedDict

parser = argparse.ArgumentParser()
parser.add_argument("config")
parser.add_argument("config_header_path")
parser.add_argument("out_path")
parser.add_argument("--print_deps", action="store_true")
parser.add_argument("--print", action="store_true")
parser.add_argument("--generate_files", action="store_true")
parser.add_argument("--group", type=int)
args = parser.parse_args()

pyconfig = args.config
config_path = args.config_header_path
out_path = args.out_path

def eval_config(path, globs):
    file0 = open(path, 'r')
    content = file0.read()
    file0.flush()
    file0.close()
    exec(content, globs)

def config_match(pattern):
    textfile = open(config_path, 'r')
    filetext = textfile.read()
    textfile.close()
    pattern = re.compile(pattern)
    for line in filetext.splitlines():
        for match in re.finditer(pattern, line):
            return True
    return False

kinds = []
def generate_code(lkinds):
    global kinds
    kinds = lkinds

eval_config(pyconfig, {
    "config_match"  : config_match,
    "generate_code" : generate_code,
})

def iprint(*arg):
    if args.print: print(*arg)

# //////////////////////////////////////////////////////////////// #
# //                                                            // #
# //  The registered algorithms now live in registry_config.py  // #
# //                                                            // #
# //////////////////////////////////////////////////////////////// #

def code(s, i = 0, r = {}):
    s = indent(dedent(s)[1:], '    ' * i)
    for key in r:
        s = s.replace(key, r[key])
    return s

class Code:
    def __init__(self):
        self.s = ""
    def code(self, s, i = 0, r = {}):
        self.s += code(s, i, r)
    def emptyline(self):
        self.s += "\n"
    def str(self):
        return self.s

algorithms_cpp_head = '''
    /* Autogenerated file by genregistry.py */
'''

def root_cpp(kinds):
    r = Code()

    # Header of root.cpp file
    r.code(algorithms_cpp_head)
    r.emptyline()
    r.code('''
        #include <tudocomp_driver/Registry.hpp>

        namespace tdc_algorithms {
            using namespace tdc;
    ''')
    r.emptyline()

    # Generate calls to each template expansion of a kind of Registry
    for (type, calls) in kinds:
        ident = type.lower()
        const = type.upper()

        # Declare and define the registry and register function
        r.code('''
            void register_$IDENTs(Registry<$TYPE>& r);
            Registry<$TYPE> $CONST_REGISTRY = Registry<$TYPE>::with_all_from(register_$IDENTs, "$IDENT");
        ''', 1, { "$TYPE": type, "$IDENT": ident, "$CONST": const })
        r.emptyline()

        # Forward-declare all template expansion calls
        for call in calls:
            r.code('''
                void register_$CALL(Registry<$TYPE>& r);
            ''', 1, { "$CALL": call, "$TYPE": type })
        r.emptyline()

        # Define the register functions
        r.code('''
            void register_$IDENTs(Registry<$TYPE>& r) {
        ''', 1, { "$TYPE": type, "$IDENT": ident, "$CONST": const })
        for call in calls:
            r.code('''
                register_$CALL(r);
            ''', 2, { "$CALL": call})
        r.code('''
            } // register_$IDENTs
        ''', 1, { "$TYPE": type, "$IDENT": ident, "$CONST": const })
        r.emptyline()
    r.code('''
        } // namespace
    ''')
    return r.str()

def single_expansion_cpp(kind, call_ident, call_type, headers):
    r = Code()

    # Header of *.cpp file
    r.code(algorithms_cpp_head)
    r.emptyline()
    r.code('''
        #include <tudocomp_driver/Registry.hpp>
    ''')
    for header in headers:
        r.code('''
            #include <tudocomp/$HEADER>
        ''', 0, { "$HEADER": header })
    r.code('''

        namespace tdc_algorithms {
            using namespace tdc;
    ''')
    r.emptyline()

    # Call with actual template expansion
    r.code('''
        void register_$CALL(Registry<$TYPE>& r) {
            r.register_algorithm<$CTYPE>();
        }
    ''', 1, { "$TYPE": kind, "$CALL": call_ident, "$CTYPE": call_type })
    r.code('''
        }
    ''')
    return r.str()

# Generates carthesian product of template params
def gen_list(ls):
    def expand_deps(algorithm):
        name = algorithm[0]
        header = algorithm[1]
        deps = algorithm[2]

        deps_lists = [gen_list(x) for x in deps]

        #pprint.pprint(deps_lists)

        deps_lists_prod = list(itertools.product(*deps_lists))

        #pprint.pprint(deps_lists_prod)

        return_list = []
        for deps_tuple in deps_lists_prod:
            if len(deps_tuple) == 0:
                return_list.append(([header], str.format("{}", name), [name]))
            else:
                hs = []
                ds = []
                hiers = []
                for (hss, d, hier) in deps_tuple:
                    for h in hss:
                        hs.append(h)
                    ds.append(d)
                    hiers += hier
                hs.append(header)
                hs = list(OrderedDict.fromkeys(hs))
                return_list.append((hs, str.format("{}<{}>", name, ",".join(ds)), [name] + hiers))

        assert len(return_list) != 0

        return return_list

    return_list = []
    for algorithm in ls:
        return_list += expand_deps(algorithm)
    return return_list

def make_hash(s):
    return hashlib.sha256(s.encode("utf-8")).hexdigest()

def update_file(path, content):
    if os.path.exists(path):
        file2 = open(path, 'r')
        existing_content = file2.read()
        file2.close()
        if existing_content == content:
            #print("Skipping", path)
            return
    file1 = open(path, 'w+')
    file1.write(content)
    file1.flush()
    file1.close()
    while not os.path.exists(path):
        time.sleep(0.01)
    while True:
        file3 = open(path, 'r')
        actually_written_content = file3.read()
        file3.close()
        if actually_written_content == content:
            break

# Output algorithm.cpp
def gen_algorithm_cpp():
    Instance = collections.namedtuple("Instance", [
        "identifier",
        "hierachy",
        "file_name",
        "content",
    ])

    kind_instances = []

    for (kind, l) in kinds:
        instances = []
        for (headers, line, hierachy) in gen_list(l):
            hsh = make_hash(line)[0:10]
            escaped_line = line \
                .replace('<', '_') \
                .replace('>', '_') \
                .replace(',', '_') \
                .replace(':', '_')
            escaped_hash_line =  hsh + "_" + escaped_line
            #print(escaped_hash_line, len(escaped_hash_line))

            path = os.path.join(out_path, escaped_hash_line + ".cpp")
            #print(path)

            instance_content = single_expansion_cpp(kind, escaped_hash_line, line, headers)

            instances.append(Instance(
                escaped_hash_line,
                hierachy,
                path,
                instance_content
            ))
        kind_instances.append((kind, instances))

    for (kind, instances) in kind_instances:
        for instance in instances:
            iprint(instance.hierachy)
            iprint(instance.file_name)
            iprint(instance.content)

            if args.generate_files:
                update_file(instance.file_name, instance.content)

    root_content = root_cpp(list(map(lambda t: (t[0], list(map(lambda x: x.identifier, t[1]))), kind_instances)))
    root_file =  os.path.join(out_path, "root" + ".cpp")
    iprint(root_file)
    iprint(root_content)
    if args.generate_files:
        update_file(root_file, root_content)

    dep_paths = [root_file]

    for (kind, instances) in kind_instances:
        if not args.group or len(instances) < args.group:
            dep_paths += list(map(lambda x: x.file_name, instances))
        else:
            instance_groups = [instances]
            while len(instance_groups) < args.group:
                first = instance_groups[0]
                if len(first) == 1:
                    break
                instance_groups.pop(0)

                counter = OrderedDict()
                for x in first:
                    for b in x.hierachy:
                        if not b in counter: counter[b] = 0
                        counter[b] += 1

                counter_l = []
                for k in counter:
                    counter_l.append((k, counter[k]))

                counter_l.sort(key=itemgetter(1))
                counter_l.reverse()

                while counter_l[0][1] == len(first):
                    counter_l.pop(0)
                #pprint.pprint(counter_l)

                head = []
                tail = []
                for ins in first:
                    if counter_l[0][0] in ins.hierachy:
                        head.append(ins)
                    else:
                        tail.append(ins)
                instance_groups.append(head)
                instance_groups.append(tail)

                #pprint.pprint(list(map(lambda y: list(map(lambda x: x.file_name, y)), instance_groups)))

                instance_groups.sort(key=lambda x: len(x))
                instance_groups.reverse()

                #print("Current grouping:")
                #pprint.pprint([len(x) for x in instance_groups])

            group_paths = []
            for group in instance_groups:
                conc = "".join([x.identifier for x in group])
                group_file_name = "group_" + make_hash(conc) + ".cpp"
                group_content = "".join([x.content for x in group])
                group_file_path = os.path.join(out_path, group_file_name)
                group_paths.append(group_file_path)

                iprint(group_file_name)
                iprint(group_content)
                if args.generate_files:
                    update_file(group_file_path, group_content)

            dep_paths += group_paths

    if args.print_deps:
        sys.stdout.write(";".join(dep_paths))
        sys.stdout.flush()


gen_algorithm_cpp()

os.sync()
