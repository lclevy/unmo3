
unmo3 (opensource) v0.6
-----------------------
19 Jul 2009


Copyright Laurent Clévy (lclevy@free.fr)
This code is under GPLv2 license. 

------------------------------------------

* changelog

- 19jul2009, code source updated to v0.6 for MO3 v2.4 encoder
  . version = 5. Also able to handle version = 3 (encoder 2.2)
  . compressed header at offset 12 instead of 8
  . 2 times bytes 0 after samples names instead of 1.
  . specification updated to v0.91
- 26feb2006, initial version : v0.5

------------------------------------------

* Introduction

The piece of code has been written as a compagnion (validation code) of the document "the unofficial MO3 specification".
See http://lclevy.free.fr/mo3
It is targeted to developpers or technical people, not for end users. It can be used by IT/XM/S3M modules
specialists (tracker editor developper or modules players) to write a MO3 import loader, or more generally 
to handle MO3 modules in any way.

The MO3 format has been created by Ian Luck (http://www.un4seen.com).
If you are looking for a good encoder and decoder (but without the source code) and a good module player,
Ian's web site is the right place to go.  

* Features of unmo3 (opensource version)

Here they are:
- uncompress the MO3 header and samples with lossless compression
- able to save uncompressed header and samples
- able to extract mp3 and ogg compressed samples
- can display a channel of a given pattern into 2 forms
	- as encoded inside MO3 file
	- as it is usually appears in a tracker editor

This code has been written under Cygwin/IA32, should work under Linux/IA32, and is supposed portable under other architectures.
It will be tested under Solaris/Ultrasparc.

If you want to run the auto tests, you have to download "unmo3_test.zip".

* Syntax

unmo3 [options] filename.mo3

available options are:

-a parselevel (from 1-4)
Display content of the MO3 file with more (-a 4) or less (-a 1) details

-d debuglevel (2)
Display some inner-working information

-v pattern_number voice_number
Display a channel of a given pattern as encoded inside MO3 module (technical output)

-o 
Must be combined with -v. Display a given channel, but as seen in any tracker editor (user friendly output).

-h header_output_filename
Write the uncompressed MO3 header into a file, for further study for example.

-s sample_number | all
Save one sample, or all samples of the MO3 module.

* Usage

- to build the executable

#make dep
#make

- demo
you can try 
#make demo
to see a 'demo'

- tests
#make test
for the auto tests : mainly to check the decompression routines ("unmo3_test.zip" archive is required).


* Not provided with this code

There is remaining work to do to interpret how all IT/XM and S3M effects and samples/instruments parameters
are stored AND interpreted by a player. But 80-90% of this work has been done in the documentation.

And maybe in a future release I'll write a .mod module writer.


Have fun.

Laurent
