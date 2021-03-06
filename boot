#!/usr/bin/perl -w

use strict;

use Cwd;
use File::Path 'rmtree';
use File::Basename;

my %required_tag;
my $validate;
my $curdir;

$required_tag{"-"} = 1;
$validate = 0;

$curdir = &cwd()
    or die "Can't find current directory: $!";

while ($#ARGV ne -1) {
    my $arg = shift @ARGV;

    if ($arg =~ /^--required-tag=(.*)/) {
        $required_tag{$1} = 1;
    }
    elsif ($arg =~ /^--validate$/) {
        $validate = 1;
    }
    else {
        die "Bad arg: $arg";
    }
}

sub sanity_check_line_endings {
    local $/ = undef;
    open FILE, "packages" or die "Couldn't open file: $!";
    binmode FILE;
    my $string = <FILE>;
    close FILE;

    if ($string =~ /\r/) {
        print STDERR <<EOF;
Found ^M in packages.
Perhaps you need to run
    git config --global core.autocrlf false
and re-check out the tree?
EOF
        exit 1;
    }
}

sub sanity_check_tree {
    my $tag;
    my $dir;

    # Check that we have all boot packages.
    open PACKAGES, "< packages";
    while (<PACKAGES>) {
        if (/^#/) {
            # Comment; do nothing
        }
        elsif (/^([a-zA-Z0-9\/.-]+) +([^ ]+) +[^ ]+ +[^ ]+$/) {
            $dir = $1;
            $tag = $2;

            # If $tag is not "-" then it is an optional repository, so its
            # absence isn't an error.
            if (defined($required_tag{$tag})) {
                # We would like to just check for a .git directory here,
                # but in an lndir tree we avoid making .git directories,
                # so it doesn't exist. We therefore require that every repo
                # has a LICENSE file instead.
                if (! -f "$dir/LICENSE") {
                    print STDERR "Error: $dir/LICENSE doesn't exist.\n";
                    die "Maybe you haven't done './sync-all get'?";
                }
            }
        }
        else {
            die "Bad line in packages file: $_";
        }
    }
    close PACKAGES;
}

# Create libraries/*/{ghc.mk,GNUmakefile}
sub boot_pkgs {
    my @library_dirs = ();
    my @tarballs = glob("libraries/tarballs/*");

    my $tarball;
    my $package;
    my $stamp;

    for $tarball (@tarballs) {
        $package = $tarball;
        $package =~ s#^libraries/tarballs/##;
        $package =~ s/-[0-9.]*(-snapshot)?\.tar\.gz$//;

        # Sanity check, so we don't rmtree the wrong thing below
        if (($package eq "") || ($package =~ m#[/.\\]#)) {
            die "Bad package name: $package";
        }

        if (-d "libraries/$package/_darcs") {
            print "Ignoring libraries/$package as it looks like a darcs checkout\n"
        }
        elsif (-d "libraries/$package/.git") {
            print "Ignoring libraries/$package as it looks like a git checkout\n"
        }
        else {
            if (! -d "libraries/stamp") {
                mkdir "libraries/stamp";
            }
            $stamp = "libraries/stamp/$package";
            if ((! -d "libraries/$package") || (! -f "$stamp")
             || ((-M "libraries/stamp/$package") > (-M $tarball))) {
                print "Unpacking $package\n";
                if (-d "libraries/$package") {
                    &rmtree("libraries/$package")
                        or die "Can't remove libraries/$package: $!";
                }
                mkdir "libraries/$package"
                    or die "Can't create libraries/$package: $!";
                system ("sh", "-c", "cd 'libraries/$package' && { cat ../../$tarball | gzip -d | tar xf - ; } && mv */* .") == 0
                    or die "Failed to unpack $package";
                open STAMP, "> $stamp"
                    or die "Failed to open stamp file: $!";
                close STAMP
                    or die "Failed to close stamp file: $!";
            }
        }
    }

    for $package (glob "libraries/*/") {
        $package =~ s/\/$//;
        my $pkgs = "$package/ghc-packages";
        if (-f $pkgs) {
            open PKGS, "< $pkgs"
                or die "Failed to open $pkgs: $!";
            while (<PKGS>) {
                chomp;
                s/\r//g;
                if (/.+/) {
                    push @library_dirs, "$package/$_";
                }
            }
        }
        else {
            push @library_dirs, $package;
        }
    }

    for $package (@library_dirs) {
        my $dir = &basename($package);
        my @cabals = glob("$package/*.cabal");
        if ($#cabals > 0) {
            die "Too many .cabal file in $package\n";
        }
        if ($#cabals eq 0) {
            my $cabal = $cabals[0];
            my $pkg;
            my $top;
            if (-f $cabal) {
                $pkg = $cabal;
                $pkg =~ s#.*/##;
                $pkg =~ s/\.cabal$//;
                $top = $package;
                $top =~ s#[^/]+#..#g;
                $dir = $package;
                $dir =~ s#^libraries/##g;

                print "Creating $package/ghc.mk\n";
                open GHCMK, "> $package/ghc.mk"
                    or die "Opening $package/ghc.mk failed: $!";
                print GHCMK "${package}_PACKAGE = ${pkg}\n";
                print GHCMK "${package}_dist-install_GROUP = libraries\n";
                print GHCMK "\$(if \$(filter ${dir},\$(PKGS_THAT_BUILD_WITH_STAGE0)),\$(eval \$(call build-package,${package},dist-boot,0)))\n";
                print GHCMK "\$(eval \$(call build-package,${package},dist-install,\$(if \$(filter ${dir},\$(PKGS_THAT_BUILD_WITH_STAGE2)),2,1)))\n";
                close GHCMK
                    or die "Closing $package/ghc.mk failed: $!";

                print "Creating $package/GNUmakefile\n";
                open GNUMAKEFILE, "> $package/GNUmakefile"
                    or die "Opening $package/GNUmakefile failed: $!";
                print GNUMAKEFILE "dir = ${package}\n";
                print GNUMAKEFILE "TOP = ${top}\n";
                print GNUMAKEFILE "include \$(TOP)/mk/sub-makefile.mk\n";
                print GNUMAKEFILE "FAST_MAKE_OPTS += stage=0\n";
                close GNUMAKEFILE
                    or die "Closing $package/GNUmakefile failed: $!";
            }
        }
    }
}

# autoreconf everything that needs it.
sub autoreconf {
    my $dir;

    foreach $dir (".", glob("libraries/*/")) {
        if (-f "$dir/configure.ac") {
            print "Booting $dir\n";
            chdir $dir or die "can't change to $dir: $!";
            system("autoreconf") == 0
                or die "Running autoreconf failed with exitcode $?";
            chdir $curdir or die "can't change to $curdir: $!";
        }
    }
}

sub checkBuildMk {
    if ($validate eq 0 && ! -f "mk/build.mk") {
        print <<EOF;

WARNING: You don't have a mk/build.mk file.

By default a standard GHC build will be done, which uses optimisation
and builds the profiling libraries. This will take a long time, so may
not be what you want if you are developing GHC or the libraries, rather
than simply building it to use it.

For information on creating a mk/build.mk file, please see:
    http://hackage.haskell.org/trac/ghc/wiki/Building/Using#Buildconfiguration

EOF
    }
}

&sanity_check_line_endings();
&sanity_check_tree();
&boot_pkgs();
&autoreconf();
&checkBuildMk();

