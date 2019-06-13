.. _xbcloud_binary:

================================================================================
The xbcloud Binary
================================================================================

.. note::

The purpose of |xbcloud| is to download and upload full or part of |xbstream|
archive from/to cloud. |xbcloud| will not overwrite the backup with the same
name. |xbcloud| accepts input via a pipe from |xbstream| so that it can be
invoked as a pipeline with |xtrabackup| to stream directly to the cloud without
needing a local storage.

|xbcloud| stores each chunk as a separate object with a name
``backup_name/database/table.ibd.NNNNNNNNNNNNNNNNNNNN``, where ``NNN...`` is a
0-padded serial number of chunk within file. Size of chunk produced by
|xtrabackup| and |xbstream| changed to 10M.

|xbcloud| has three essential operations: *put*, *get*, and *delete*. With these
operations, backups are created, stored, retrieved, restored, and
deleted. |xbcloud| operations clearly map to similar operations within the AWS
S3 API.

Supported Cloud Storage Types
================================================================================

In addition to Swift, which has been the only option for storing backups in a
cloud storage until |Percona XtraBackup| 2.4.14, |xbcloud| supports *Amazon S3*,
*MinIO*, and *Google Cloud Storage*. Other Amazon S3 compatible storages, such
as Wasabi or Digital Ocean Spaces, are also supported.

.. seealso::

   OpenStack Object Storage ("Swift")
      https://wiki.openstack.org/wiki/Swift
   Amazon Simple Storage Service
      https://aws.amazon.com/s3/
   MinIO
      https://min.io/
   Google Cloud Storage
      https://cloud.google.com/storage/
   Wasabi
      https://wasabi.com/
   Digital Ocean Spaces
      https://www.digitalocean.com/products/spaces/

Usage
================================================================================

.. code-block:: bash

   $ xtrabackup --backup --stream=xbstream --target-dir=/tmp | xbcloud \
   put [options] <name>

Creating a full backup
--------------------------------------------------------------------------------

.. rubric:: Swift

The following example shows how to make a full backup and upload it to Swift

.. code-block:: bash

   $ xtrabackup --backup --stream=xbstream --extra-lsndir=/tmp --target-dir=/tmp | \
   xbcloud put --storage=swift \
   --swift-container=test \
   --swift-user=test:tester \
   --swift-auth-url=http://192.168.8.80:8080/ \
   --swift-key=testing \
   --parallel=10 \
   full_backup

.. rubric:: Amazon S3

.. code-block:: bash

   $ xtrabackup --backup --stream=xbstream --extra-lsndir=/tmp --target-dir=/tmp | \
   xbcloud put --storage=s3 \
   --s3-endpoint='s3.amazonaws.com' \
   --s3-access-key='YOUR-ACCESSKEYID' \
   --s3-secret-key='YOUR-SECRETACCESSKEY' \
   --s3-bucket='mysql_backups'
   --parallel=10 \
   ${date}-full_backup

The following options are available when using Amazon S3:

.. list-table::
   :header-rows: 1

   * - Option
     - Details
   * - --s3-access-key
     - Use to supply the AWS access key ID
   * - --s3-secret-key
     - Use to supply the AWS secret access key
   * - --s3-bucket
     - Use supply the AWS bucket name
   * - --s3-region
     - Use to specify the AWS region. The default value is **us-east-1**
   * - --s3-api-version = <AUTO|2|4>
     - Select the signing algorithm. The default value is AUTO. In this case, |xbcloud| will probe.
   * - --s3-bucket-lookup = <AUTO|PATH|DNS>
     - Specify whether to use **bucket.endpoint.com** or *endpoint.com/bucket**
       style requests. The default value is AUTO. In this case, |xbcloud| will probe.

.. rubric:: MinIO
	    
.. code-block:: bash

   $ xtrabackup --backup --stream=xbstream --extra-lsndir=/tmp --target-dir=/tmp | \
   xbcloud put --storage=s3 \
   --s3-endpoint='play.minio.io:9000' \
   --s3-access-key='YOUR-ACCESSKEYID' \
   --s3-secret-key='YOUR-SECRETACCESSKEY' \
   --s3-bucket='mysql_backups'
   --parallel=10 \
   ${date}-full_backup

.. rubric:: |gcs|

The support for |gcs| is implemented using the interoperability
mode. This mode was especially designed to interact with cloud services
compatible with Amazon S3.

.. seealso::

   Cloud Storage Interoperability
      https://cloud.google.com/storage/docs/interoperability

.. code-block:: bash
		
   $ xtrabackup --backup --stream=xbstream --extra-lsndir=/tmp --target-dir=/tmp | \
   xbcloud put --storage=google \
   --google-endpoint=`storage.googleapis.com` \
   --google-access-key='YOUR-ACCESSKEYID' \
   --google-secret-key='YOUR-SECRETACCESSKEY' \
   --google-bucket='mysql_backups'
   --parallel=10 \
   ${date}-full_backup

The following options are available when using |gcs|:

.. hlist::
   :columns: 2

   - --google-access-key = <ACCESS KEY ID>
   - --google-secret-key = <SECRET ACCESS KEY>
   - --google-bucket = <BUCKET NAME>

Supplying parameters
--------------------------------------------------------------------------------

Each storage type has mandatory parameters that you can supply on the command
line, in a configuration file, and via environment variables.

.. rubric:: Configuration files

The parameters the values of which do not change frequently can be stored in
:file:`my.cnf` or in a custom configuration file. The following example is a
template of configuration options under the [xbcloud] group:

.. code-block:: text

   [xbcloud]
   storage=s3
   s3-endpoint=http://localhost:9000/
   s3-access-key=minio
   s3-secret-key=minio123
   s3-bucket=backupsx
   s3-bucket-lookup=path
   s3-api-version=4

.. note::

   If you explicitly use a parameter on the command line and in a configuration
   file, |xbcloud| uses the the value provided on the command line.

.. rubric:: Environment variables

The following environment variables are recognized. |xbcloud| maps them
automatically to corresponding parameters applicable to the selected storage.

.. hlist::
   :columns: 2

   - AWS_ACCESS_KEY_ID (or ACCESS_KEY_ID)
   - AWS_SECRET_ACCESS_KEY (or SECRET_ACCESS_KEY)
   - AWS_DEFAULT_REGION (or DEFAULT_REGION)
   - AWS_ENDPOINT (or ENDPOINT)
   - AWS_CA_BUNDLE

.. node::

   If you explicitly use a parameter on the command line, in a configuration
   file, and the corresponding environment variable contains a value, |xbcloud|
   uses the the value provided on the command line or in the configuration file.

OpenStack environment variables are also recognized and mapped automatically to
corresponding **swift** parameters (``--storage=swift``).

.. hlist::
   :columns: 2

   - OS_AUTH_URL
   - OS_TENANT_NAME
   - OS_TENANT_ID
   - OS_USERNAME
   - OS_PASSWORD
   - OS_USER_DOMAIN
   - OS_USER_DOMAIN_ID
   - OS_PROJECT_DOMAIN
   - OS_PROJECT_DOMAIN_ID
   - OS_REGION_NAME
   - OS_STORAGE_URL
   - OS_CACERT

.. rubric:: Shortcuts

For all operations (put, get, and delete), you can use a shortcut to specify the
storage type, bucket name, and backup name as one parameter instead of using
three distinct parameters (--storage, --s3-bucket, and backup name per se).

.. admonition:: Using a shortcut syntax to provide a storage type, bucket, and backup name

   Use the following format: ``storage-type://bucket-name/backup-name``

   .. code-block:: bash

      $ xbcloud get s3://operator-testing/bak22 ...

   In this example, **s3** refers to a storage type, **operator-testing** is a
   bucket name, and **bak22** is the backup name. This shortcut expands as
   follows:

   .. code-block:: bash

      $ xbcloud get --storage=s3 --s3-bucket=operator-testing bak22 ...

You can supply the mandatory parameters not only on the command line. You may use
configuration files and environment variables.

.. rubric:: Additional parameters

|xbcloud| accepts additional parameters that you can use with any storage
type. The ``--md5`` parameter computes the MD5 hash value of the backup
chunks. The result is stored in files that following the ``backup_name.md5``
pattern.

.. code-block:: bash

   $ xtrabackup --backup --stream=xbstream \
   --parallel=8 2>backup.log | xbcloud put s3://operator-testing/bak22 \
   --parallel=8 --md5 2>upload.log

You may use the ``--header`` parameter to pass an additional HTTP
header with the server side encryption while specifying a customer key.

.. admonition:: Example of using --header for AES256 encryption

   .. code-block:: bash

      $ xtrabackup --backup --stream=xbstream --parallel=4 | \
      xbcloud put s3://operator-testing/bak-enc/ \
      --header="X-Amz-Server-Side-Encryption-Customer-Algorithm: AES256" \
      --header="X-Amz-Server-Side-Encryption-Customer-Key: CuStoMerKey=" \
      --header="X-Amz-Server-Side-Encryption-Customer-Key-MD5: CuStoMerKeyMd5==" \
      --parallel=8

The ``--header`` parameter is also useful to set the access control list (ACL)
permissions: ``--header="x-amz-acl: bucket-owner-full-control``

Restoring with Swift
--------------------------------------------------------------------------------

.. rubric:: Swift

.. code-block:: bash

   $ xbcloud get [options] <name> [<list-of-files>] | xbstream -x

The following example shows how to fetch and restore the backup from Swift:

.. code-block:: bash

   $ xbcloud get --storage=swift \
   --swift-container=test \
   --swift-user=test:tester \
   --swift-auth-url=http://192.168.8.80:8080/ \
   --swift-key=testing \
   full_backup | xbstream -xv -C /tmp/downloaded_full

   $ xbcloud delete --storage=swift --swift-user=xtrabackup \
   --swift-password=xtrabackup123! --swift-auth-version=3 \
   --swift-auth-url=http://openstack.ci.percona.com:5000/ \
   --swift-container=mybackup1 --swift-domain=Default

.. rubric:: Amazon S3

.. code-block:: bash

   $ xbcloud get s3://operator-testing/bak22 \
   --s3-endpoint=https://storage.googleapis.com/ \
   --parallel=10 2>download.log | xbstream -x -C restore --parallel=8

Incremental backups
================================================================================

.. rubric:: Taking incremental backups

First you need to make the full backup on which the incremental one is going to
be based:

.. code-block:: bash

  xtrabackup --backup --stream=xbstream --extra-lsndir=/storage/backups/ \
  --target-dir=/storage/backups/ | xbcloud put \
  --storage=swift --swift-container=test_backup \
  --swift-auth-version=2.0 --swift-user=admin \
  --swift-tenant=admin --swift-password=xoxoxoxo \
  --swift-auth-url=http://127.0.0.1:35357/ --parallel=10 \
  full_backup

Then you can make the incremental backup:

.. code-block:: bash

   $ xtrabackup --backup --incremental-basedir=/storage/backups \
   --stream=xbstream --target-dir=/storage/inc_backup | xbcloud put \
   --storage=swift --swift-container=test_backup \
   --swift-auth-version=2.0 --swift-user=admin \
   --swift-tenant=admin --swift-password=xoxoxoxo \
   --swift-auth-url=http://127.0.0.1:35357/ --parallel=10 \
   inc_backup

.. rubric:: Preparing incremental backups

To prepare a backup you first need to download the full backup:

.. code-block:: bash

   $ xbcloud get --swift-container=test_backup \
   --swift-auth-version=2.0 --swift-user=admin \
   --swift-tenant=admin --swift-password=xoxoxoxo \
   --swift-auth-url=http://127.0.0.1:35357/ --parallel=10 \
   full_backup | xbstream -xv -C /storage/downloaded_full

Once you download the full backup it should be prepared:

.. code-block:: bash

   $ xtrabackup --prepare --apply-log-only --target-dir=/storage/downloaded_full

After the full backup has been prepared you can download the incremental
backup:

.. code-block:: bash

   $ xbcloud get --swift-container=test_backup \
   --swift-auth-version=2.0 --swift-user=admin \
   --swift-tenant=admin --swift-password=xoxoxoxo \
   --swift-auth-url=http://127.0.0.1:35357/ --parallel=10 \
   inc_backup | xbstream -xv -C /storage/downloaded_inc

Once the incremental backup has been downloaded you can prepare it by running:

.. code-block:: bash

   $ xtrabackup --prepare --apply-log-only \
   --target-dir=/storage/downloaded_full \
   --incremental-dir=/storage/downloaded_inc

   $ xtrabackup --prepare --target-dir=/storage/downloaded_full

Partial download of the cloud backup
================================================================================

If you do not want to download the entire backup to restore the specific
database you can specify only tables you want to restore:

.. code-block:: bash

   $ xbcloud get --swift-container=test_backup
   --swift-auth-version=2.0 --swift-user=admin \
   --swift-tenant=admin --swift-password=xoxoxoxo \
   --swift-auth-url=http://127.0.0.1:35357/ full_backup \
   ibdata1 sakila/payment.ibd \
   > /storage/partial/partial.xbs
 
   $ xbstream -xv -C /storage/partial < /storage/partial/partial.xbs
 
This command will download just ``ibdata1`` and ``sakila/payment.ibd`` table
from the full backup.

Command-line options
================================================================================

|xbcloud| has the following command line options:

.. option:: --storage=[swift|s3|google]

   Cloud storage option. |xbcloud| supports Swift, MinIO, and AWS S3.
   The default value is ``swift``.

.. option:: --swift-auth-url

   URL of Swift cluster.

.. option:: --swift-storage-url

   xbcloud will try to get object-store URL for given region (if any specified)
   from the keystone response. One can override that URL by passing
   --swift-storage-url=URL argument.

.. option:: --swift-user

   Swift username (X-Auth-User, specific to Swift)

.. option:: --swift-key

   Swift key/password (X-Auth-Key, specific to Swift)

.. option:: --swift-container

   Container to backup into (specific to Swift)

.. option:: --parallel=N

   Maximum number of concurrent upload/download threads. Default is ``1``.

.. option:: --cacert

   Path to the file with CA certificates

.. option:: --insecure

   Do not verify servers certificate

.. _swift_auth:

Swift authentication options
----------------------------

Swift specification describes several `authentication options
<http://docs.openstack.org/developer/swift/overview_auth.html>`_. |xbcloud| can
authenticate against keystone with API version 2 and 3.

.. option:: --swift-auth-version

   Specifies the swift authentication version. Possible values are: ``1.0`` -
   TempAuth, ``2.0`` - Keystone v2.0, and ``3`` - Keystone v3. Default value is
   ``1.0``.

For v2 additional options are:

.. option:: --swift-tenant

   Swift tenant name.

.. option:: --swift-tenant-id

   Swift tenant ID.

.. option:: --swift-region

   Swift endpoint region.

.. option:: --swift-password

   Swift password for the user.

For v3 additional options are:

.. option:: --swift-user-id

   Swift user ID.

.. option:: --swift-project

   Swift project name.

.. option:: --swift-project-id

   Swift project ID.

.. option:: --swift-domain

   Swift domain name.

.. option:: --swift-domain-id

   Swift domain ID.

.. |gcs| replace:: Google Cloud Storage
