2022-12-15, Release Steps for ooRexx 5.0.0 as a role model
2025-04-29, Updated for 5.1.0 release
2025-05-01/03, Updated for 5.1.0 release
2026-03-14, Updated for 5.2.0 release
===

Step 1.
-------
- create a branch for `5.2.0` using current trunk (if no show stopper errors occur
  running the ooRexx testsuite) for
      `main/trunk` to `main/branches` and
      `test/trunk` to `test/branches`,
      not yet for the docs (see below)

  here the svn command for code and test (replace "userId" with yours)

      svn copy --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/main/trunk svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/main/branches/5.2.0/trunk  -m "Creating code branch 5.2.0 to prepare release."
      svn copy --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/test/trunk svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/test/branches/5.2.0/trunk -m "Creating test branch 5.2.0 to prepare release."

  alternative (locally from oorexx/code-0 directory):

      svn mkdir main/branches/5.2.0
      svn copy main/trunk main/branches/5.2.0
      svn ci -m "Creating code branch 5.2.0 to prepare release."

      svn mkdir test/branches/5.2.0
      svn copy test/trunk test/branches/5.2.0
      svn ci -m "Creating test branch 5.2.0 to prepare release."


- *before* creating the docs `docs/branches/5.2.0/trunk`, the necessary steps:

  - make sure all authors who have committed in the meantime are listed in their appropriate
    `${book}/en-US/Author_Group.xml`; to detect one can use `tools/listAuthorsAsOfLate.rex`

  - make sure that `${book}/revision_info.txt` has the latest revision information
    of real updates to `${book}/en-US/*.xml` files; one approach to update that file
    is to first delete it, then run `updateEntityValues.rex -y 2026 -e "2026.03.14" -r 99999 ..`
    which will recreate `${book}/revision_info.txt`

  - make sure that all copyright texts are updated (you can use `tools/updateCopyright.rex`)

  - *NOTE:* copyright of `main/branches/5.2.0/trunk/ReleaseNotes` needs to be adjusted as well!

- create the docs branch for `5.2.0` using current `docs/trunk` to `docs/branches/5.2.0/trunk`

  here the svn command for code, documentation and test (replace "userId" with yours)

      svn copy --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/docs/trunk svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/docs/branches/5.2.0/trunk -m "Creating docs branch 5.2.0 to prepare release."

  alternative (locally from oorexx/code-0 directory):

      svn mkdir docs/branches/5.2.0
      svn copy docs/trunk docs/branches/5.2.0
      svn ci -m "Creating docs branch 5.2.0 to prepare release."


These steps are performed mainly using a script that will replace the xml files defining all jobs on the Jenkins Controller
There is a step-by-step list to follow for this phase.

- adjust release information:

  - update `main/branches/5.2.0/NOTICE` (a text file) to reflect the current copyright year

  - update `main/branches/5.2.0./CHANGES` (a text file) to document all changes from the last release for

       - Bugs
       - Feature-requests
       - Documentation
       - Patches

    1) all items with the status `accepted` need to be checked to see whether they
       have been worked on such that they can be changed to `pending`.
       `accepted` generally means code changes have been made, so these should be
       included in the changes.

       A decision needs to be made on whether the incomplete portions of the item
       (generally doc or test work) need to be completed prior to the release.

    2) all items with the status `pending` and `accepted` get included in `CHANGES`
       (the text file located in trunk), the script

       `main/trunk/support/sourceForge/processedTickets.rex`

       queries the four categories on SourceForge and creates the text for the entries
       to be added to `CHANGES`

    3) change all tickets with the status `pending` and `accepted` with no pending work items
       to `closed` using `bulk-edit` (categories: Bugs, Feature-requests, Documentation, Patches):

       - for each ticket category choose `Searches` in the left column towards the bottom

       - search/query for `pending` tickets, enter the following value in the search field:

         * category: Bugs, Feature-requests

               status:pending or (status:accepted AND _pending_work_items:none)

         * category: Documentation, Patches

               status:pending or status:accepted

       - if there are more than 25 hits, choose the maximum of 250

       - in the upper right hand corner choose `bulk-edit`

         - change status to `closed`

         - click the check box in the title underneath to select all tickets

         - click `Save`

     - repeat, if more than 250 hits

    4) a `Milestone` `5.2.0` and `5.2.1` (for potential bug fix releases) needs to be
       created, here the steps needed for `Bugs`:

       - pick `Administration`

       - click on the lock icon in the upper hand corner

       - click on the `Tickets` menu and then on the changed config "icon"
         left of the text `Bugs`

       - choose `Field Management`

       - add `5.2.0` and `5.2.1` to `Milestones` which represents ooRexx
         version numbers offered when new bugs get created; to take effect
         one needs to press the `Save` button underneath the visual frame

         *Note:* this option page would allow to delete Milestones (e.g. if no
             no tickets registered) and to declare Milestones to be "completed"
             by checking the respective box; deleted and completed Milestones
             will cause them to not be shown in the "Groups" frame on the left
             hand side.

Step 2.
-------
- update the ooRexx version related information in main/trunk to `5.2.0`

  cf. <https://sourceforge.net/p/oorexx/code-0/12539/#diff-1> changes to:

      main/trunk/CMakeLists.txt   -- update the release version information to the next release.
                section:
                        # The version of ooRexx to make
                        set (ORX_API_LEVEL 4)
                        set (ORX_MAJOR 5)
                        set (ORX_MINOR 2)
                        set (ORX_MOD_LVL 0)
                        set (ORX_BLD_LVL 0)

      main/trunk/platform/windows/rexx32.exe.manifest
      main/trunk/platform/windows/rexx64.exe.manifest

      main/trunk/api/oorexxapi.h  -- Add a new version identifier define for the new release.


- create the following directories in files area on Sourceforge:

      oorexx/files/5.2.0_Release_Candidate        ... to receive the tested builds from branches/5.2/trunk
      oorexx/oorexx-docs/5.2.0_Release_Candidate  ... to receive the new documentation from docs/branches/5.2/trunk

      oorexx/files/5.3.0beta        ... to receive the new builds from trunk
      oorexx/oorexx-docs/5.3.0beta  ... to receive the new documentation from trunk

- NOTE The following steps are done using the script that replaces the xml files defining all jobs on Jenkins
- DO NOT modify the Jenkins build jobs manually during the build process!
- adjust the build scripts on Jenkins and feed the oorexx/files/5.2.0_Release_Candidate
- adjust the build scripts on Jenkins and feed the oorexx/oorexx-docs/5.2.0_Release_Candidate

When that is complete and all artifacts have been uploaded

- NOTE The following steps are done using the script that replaces the xml files defining all jobs on Jenkins
- again DO NOT modify the Jenkins build jobs manually during the build process!
- adjust the build scripts on Jenkins and feed the `oorexx/files/5.3.0beta`
- adjust the build scripts on Jenkins and feed the `oorexx/oorexx-docs/5.3.0beta`


Step 3.
-------
- do the release:

  - rename the folders from `5.2.0_Release_Candidate` to `5.2.0`

  - Update the SourceForge default download links to point to the new releases

  - announce the new release on the ooRexx related e-mail lists,
    RexxLA e-mail list, BSF4ooRexx related e-mail lists, Open Object Rexx Facebook page,
    comp.lang.rexx

    - ask the reader to forward the announcement to related e-mail lists
      they participate (like the various IBM e-mail lists, other related
      open-source e-mail lists and projects)

  - add a release entry in the SourceForge project's "News" section using
    the link: <https://sourceforge.net/p/oorexx/news/>

  - use "svn move" to move `main/branches/5.2.0/trunk` to `main/releases/5.2.0/trunk`, 
  - do the same for the documentation `docs/branches/5.2.0/trunk` to `docs/releases/5.2.0/trunk`
  - and tests `test/branches/5.2.0/trunk` to `test/releases/5.2.0/trunk`

        svn move --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/main/branches/5.2.0/trunk svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/main/releases/5.1.0/trunk -m "Creating main/releases/5.2.0 to conclude release process."
        svn move --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/docs/branches/5.2.0/trunk svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/docs/releases/5.1.0/trunk -m "Creating docs/releases/5.2.0 to conclude release process."
        svn move --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/test/branches/5.2.0/trunk svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/test/releases/5.1.0/trunk -m "Creating test/releases/5.2.0 to conclude release process."

    alternative (locally from oorexx/code-0 directory)vely:

        svn mkdir docs/releases/5.2.0
        svn move  docs/branches/5.2/trunk docs/releases/5.2.0/trunk
        svn rm    docs/branches/5.2.0

        svn mkdir main/releases/5.2.0
        svn move  main/branches/5.2.0/trunk main/releases/5.2.0/trunk
        svn rm    main/branches/5.2.0

        svn mkdir test/releases/5.2.0
        svn move  test/branches/5.2.0/trunk test/releases/5.2.0/trunk
        svn rm    test/branches/5.2.0

        svn ci -m "Creating main/releases/5.2.0 to conclude release process."

  - change into docs/trunk/tools and run

        updateEntityValues.rex -y 2026 -v "5.2.0 -> 5.3.0" -e "2026.03.14" -r 99999 ..

  - update the ooRexx documentation in trunk to not show *CHG* and *NEW* for 51 anymore by
    setting the replacement text to the empty string `""`, from

        <!ENTITY added52 "*NEW* ">
        <!ENTITY changed52 "*CHG* ">
    to
        <!ENTITY added52 "">
        <!ENTITY changed52 "">

    and to add for new additions and changes in the documentation

        <!ENTITY added53 " *NEW* ">
        <!ENTITY changed53 " *CHG* ">

    in the files: `rexxapi/en-US/rexxapi.ent`, `rexxpg/en-US/rexxpg.ent`, `rexxref/en-US/rexxref.ent`,
    `winextensions/en-US/winextensions.ent`

  - change into main/trunk, edit CHANGES to remove all changes documented for
    5.2.0, keep the headings

  - change into `main/trunk`, edit `ReleaseNotes` to remove all changes documented for
    5.2.0, adjust copyright year and version (change from `5.1.0` to `5.2.0` and
    `5.1.0` to `5.2.0`)


Step 4.
-------
- overall timetable estimate:

  - t0: if the tests on Jenkins show no show-stopper errors from the latest
        trunk, then create the branch

  - t1: create the installation packages from the branch and upload them to a
        staged directory `5.2.0/trunk`

  - t2: apply the necessary changes in the branch (version information update,
        release information update): within (t0+7) days the longest

  - t3: communicate the availability: t2+1

  - t4: once the release has been finalized, create a x.x.(n+1) branch from the release branch
        to be used for potential bug fix releases. This includes updating the release number information
        in the build.
        move the `main|docs|test"/branches/5.2.0/trunk` to `main|docs|test"/releases/5.2.0`


Step 5.
-------
- in the case that updates are needed to the release version one needs to create
  appropriate branches: `branches/5.2.1` for `main`, `docs`, `test` to work in

*QUESTION*: is this really necessary? can we not just overwrite the /branches/5.1.0/trunk compare to version 4 releases
- Since we have MOVED /branches/5.X.0 to /releases/5.X.0 we need to recreate /branches/5.X.0 renamed as /branches/5.X.Y
- In the past branches/4.0, 4.1, 4.2 were used and COPIED OVER at release, not like today when they are moved.
  It is my (P.O.) understanding that a `branches/4.X` could serve as a starting point for all 4.X.Y minor releases, if kept.

        svn copy --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/main/releases/5.2.0/trunk svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/main/branches/5.2.1/trunk -m "Creating code branch 5.2.1."
        svn copy --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/docs/releases/5.2.0       svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/docs/branches/5.2.1/trunk -m "Creating docs branch 5.2.1."
        svn copy --username=userId svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/test/releases/5.1.0       svn+ssh://orexx@svn.code.sf.net/p/oorexx/code-0/test/branches/5.2.1/trunk -m "Creating test branch 5.2.1."


----
Helpful SourceForge resources:

- SourceForge Documentation Home:   <https://sourceforge.net/p/forge/documentation/Docs%20Home/>

- Release API:          <https://sourceforge.net/p/forge/documentation/Using%20the%20Release%20API/>

- Bulk-edit of Tickets: <https://sourceforge.net/p/forge/documentation/Tickets/#bulk-edit>

- Markdown Syntax:      <https://sourceforge.net/nf/markdown_syntax>

- Very nice and helpful introduction to SourceForge setup, including "howto" backing up:
  <https://www.software.ac.uk/resources/guides/how-create-and-manage-sourceforge-projects>



------------

Changes to be made to Jenkins are set out in a separate document here a short list of the jobs 2025-05-29

1 Documentation job:
ooRexx-docs-build

1 Source-Package job:
ooRexx-source-package-build

1 sourceforge-upload job:
ooRexx-sourceforge-upload

1 sourceforge-zip-installer-upload job:
ooRexx-sourceforge-zip-installer-upload

33 Build jobs to process:

10 Windows build jobs to process:
ooRexx-windows32-build		(Win11)
ooRexx-windows64-build		(Win11)
ooRexx-windows7_32-build
ooRexx-windows7_64-build
ooRexx-windows8_32-build    (Windows 8.1)
ooRexx-windows8_64-build    (Windows 8.1)
ooRexx-windows10_32-build
ooRexx-windows10_64-build
ooRexx-windows11_32-build
ooRexx-windows11_64-build

3 macOS build jobs to process:
ooRexx-macOS10-X86_64-build	    (High Sierra)
ooRexx-macOS15-X86_64-build		(Sequoia)
ooRexx-macOS26-M4-build         (Tahoe)

21 *nix build jobs to process:

DEB
ooRexx-Debian12-build
ooRexx-Ubuntu24-build
ooRexx-Ubuntu22-build
ooRexx-LinuxMint22-build

RPM
ooRexx-CentOS10-build
ooRexx-Fedora43-build
ooRexx-OpenSuse15-build

BSD
ooRexx-FreeBSD14-build
ooRexx-OpenBSD7-build
ooRexx-NetBSD10-build

ARCH
ooRexx-Manjaro-build
ooRexx-ArchLinux-build

UNIX
ooRexx-OpenIndiana-build
ooRexx-Solaris11-build

Raspberry PI
ooRexx-RaspberryPiOS32-build    (Raspbian GNU/Linux 11 (bullseye))
ooRexx-RaspberryPiOS64-build    (Debian GNU/Linux 11 (bullseye))
ooRexx-linux-aarch64-build

Developer
oorexx-ubuntu16-build
oorexx-ubuntu16debug-build
ooRexx-Controller-Ubuntu-build  (Ubuntu 24)
ooRexx-OpenBSD7-debug-build

Maintenance
-configuration_best_practises-
Jenkins-Environment-Variables-List
ooRexx-docs-buildutils-check
Throttling a group of jobs

10 Windows test jobs

3 macOS test jobs

20 *nix test jobs (both ubuntu16 builds use the same test job)

The following changes are made by the script Modifybuildjobs.rex located in ooRexxSVN-Code-0/build-utilities/trunk/common/release/
The script when run modify the xml files defining each jobs in /var/lib/jenkins/jobs/* It is also possible (safer) to copy the entire /var/lib/jenkins/jobs branch to a local workingplace and run the script on that path and thereafte copying back the entire branch again, overwriting the existing branch. During the copying Jenkins should be shut off

Change Build Repository URL from trunk
https://svn.code.sf.net/p/oorexx/code-0/main/trunk
to Release Candidate
https://svn.code.sf.net/p/oorexx/code-0/main/branches/5.2.0/trunk

For all Test jobs

Change Test Repository URL from trunk (not yet done)
https://svn.code.sf.net/p/oorexx/code-0/test/trunk
to Release Candidate
https://svn.code.sf.net/p/oorexx/code-0/test/branches/5.2.0/trunk

in ooRexx-docs-build

change the Documentation Repository URL from trunk
https://svn.code.sf.net/p/oorexx/code-0/docs/trunk
to the release candidate
https://svn.code.sf.net/p/oorexx/code-0/docs/branches/5.2.0/trunk

Currently the upload of all docs go to the same folder /oorexx-docs/
The PDF docs are uploaded as-is whereas the html documentation is zipped (extension .zip)

oorexx-docs/5.2.0_Release_Candidate (staged)
Warning: it seems sftp does not work for staged folders, to be sure the upload works make
the upload folder on sourceforge visible and then stage it once the upload has been completed.

ooRexx-{sourceforge-upload}
Currently the upload of  artifacts go to
sourceforgeFiles = "/home/frs/project/oorexx/oorexx/5.2.0beta"
sourceforgeFiles = "/home/frs/project/oorexx/oorexx/5.2.0_Release_Candidate" (staged)

This change will influence jenkinsArtifactUpload.rex in /var/lib/jenkins/workspace/ooRexx-{sourceforge-upload}

Before any new release go through the tools used for documentation build to make sure they are still up to date;
make any changes BEFORE a specific version is frozen to avoid double copying to release and trunk

Same for all platforms that we are currently building, work through all build and test "Projects" on Jenkins and
note any build that fail (and the reason therefor) and any tests that are excluded (with a hint to a bug report if available)
There is a section at the top of the Jenkins main schreen where these notes should be kept easily visible to all

In order to get the correct documentation added to Windows and macOS installers it is imperative to build the documentation BEFORE the final build of code is triggered. The documentation need around one hour to complete when all documents are built so within that time no build of code should be triggered. The current setup should take care of this automatically since all code build jobs are blocked until the documentation is ready but after a rebuild of the documentation has been triggered it is safest to wait until it is done before launcing the remaining jobs.

On several *nix platforms the installer build is tested after build

sudo rpm --upgrade ooRexx-5.2.0-*.rpm || true
rpm --query --info "ooRexx-5.2.0-*"
sudo rpm --erase "ooRexx-5.2.0-*"
sudo rpm --install ooRexx-5.2.0-*.rpm

BEFORE the build of the release candidate make sure to uninstall manually on all *nix platforms

TODO List for ooRexx 5 after a release is done

Remove oorexx-docs/5.2.0beta documentation on Sourceforge

Remove oorexx/5.2.0beta on Sourceforge

After release Jenkins shold go back to trunk

For Build Projects change
from https://svn.code.sf.net/p/oorexx/code-0/main/branches/5.2.0/trunk
to   https://svn.code.sf.net/p/oorexx/code-0/main/trunk

For Test Projects change
from https://svn.code.sf.net/p/oorexx/code-0/test/branches/5.2.0/trunk
to https://svn.code.sf.net/p/oorexx/code-0/test/trunk

Change Jenkins upload go to oorexx/5.3.0beta/ -> input to uploading script in upload job

Change Jenkins doc upload go to oorexx-docs/5.3.0beta/ -> in Docbuild Project

Make sure to build only AFTER documentation has been built

Problem 1:
oorexx-docs-bildutils folder on sourceforge is not automatically updated.

Temporary solution: use SFTP to upload the latest tools
Long term solution: check if this can be made a "Project" in Jenkins

Problem 2:
oorexx-buildutils is also not automatically updated. Further only Windows tools are present.

Temporary solution: use SFTP to upload the latest tools
Long term solution: a "Project" in Jenkins + Further tools for macOS and Linux/Unix

Problem 3:
In oorexx-docs/5.X.0 folders on SF the Readme.txt is missing
Solution: create a Readme.txt and try to set date to release date
