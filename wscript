#! /usr/bin/env python
# encoding: utf-8

VERSION='0.0.1'
APPNAME='tsynth'

top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')

def build(bld):
    bld.program(
        source = [
            'main.cpp',
            'synth.cpp',
            'parse.cpp',
            'tree.cpp',
            'midi_utility.cpp',
            'mono_synth.cpp',
            'mod_factory.cpp',
            'vco.cpp',
            'vca.cpp',
            'mixer.cpp',
            'vcf.cpp',
            'eg.cpp',
            'inv_exp_table.cpp',
            'alsa/alsa_pcm_out.cpp',
            'alsa/alsa_midi_in.cpp'],
        target = 'tsynth',
        lib = ['boost_thread', 'asound'],
        cxxflags = ['-O3', '-Wall', '-DNDEBUG', '-std=c++0x'],
        includes = ['.', 'alsa'])

