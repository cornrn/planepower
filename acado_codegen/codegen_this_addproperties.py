#!/usr/bin/env python

import re

def comment_remover(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return ""
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern, replacer, text)

lines = open("model_constants.hpp").readlines()
text = ''.join(lines)
text2 = comment_remover(text)
foo = [line for line in text2.split('\n') if 'Parameter' in line]
foo = [line.replace(';','') for line in foo]
foo = [line.replace('\t','') for line in foo]
foo = [line.replace('Parameter','') for line in foo]
foo = [line.replace(' ','') for line in foo]


lines = []
for line,i in zip(foo,range(len(foo))):
	lines.append('this->addProperty("%s", acadoVariables.p[ %i ]);'%(line,i))

outtext = '\n'.join(lines)
warnstr = '// WARNING!!! THIS CODE SNIPPET IS AUTOGENERATED\n'
warnstr += '//	BY acado_write_marchalling_snippet.py!!!!\n\n' 
outtext = warnstr + outtext + '\n'
outname = 'this_addproperties.h'
open(outname,'w').write(outtext)
