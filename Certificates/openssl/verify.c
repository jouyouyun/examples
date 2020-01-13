/* ------------------------------------------------------------ *
 * file:        certverify.c                                    *
 * purpose:     Example code for OpenSSL certificate validation *
 * author:      06/12/2012 Frank4DD                             *
 *                                                              *
 * gcc -o certverify certverify.c -lssl -lcrypto                *
 * ------------------------------------------------------------ */

// From
//   http://fm4dd.com/openssl/certverify.shtm
// RSA_public_decrypt failed:
//   https://stackoverflow.com/questions/44131568/rsa-public-decrypt-failed
// Programmatically verify certificate
//   https://stackoverflow.com/questions/16291809/programmatically-verify-certificate-chain-using-openssl-api
// x509证书验证
//   https://www.cnblogs.com/Dennis-mi/articles/3267299.html

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

int main(int argc, char *argv[]) {

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <root cert> <cert>\n", argv[0]);
		return 0;
	}
	const char *ca_bundlestr = argv[1];
	const char *cert_filestr = argv[2];

	BIO              *certbio = NULL;
	BIO               *outbio = NULL;
	X509          *error_cert = NULL;
	X509                *cert = NULL;
	X509_NAME    *certsubject = NULL;
	X509_STORE         *store = NULL;
	X509_STORE_CTX  *vrfy_ctx = NULL;
	int ret;

	/* ---------------------------------------------------------- *
	 * These function calls initialize openssl for correct work.  *
	 * ---------------------------------------------------------- */
	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();

	/* ---------------------------------------------------------- *
	 * Create the Input/Output BIO's.                             *
	 * ---------------------------------------------------------- */
	certbio = BIO_new(BIO_s_file());
	outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

	/* ---------------------------------------------------------- *
	 * Initialize the global certificate validation store object. *
	 * ---------------------------------------------------------- */
	if (!(store=X509_STORE_new()))
		BIO_printf(outbio, "Error creating X509_STORE_CTX object\n");

	/* ---------------------------------------------------------- *
	 * Create the context structure for the validation operation. *
	 * ---------------------------------------------------------- */
	vrfy_ctx = X509_STORE_CTX_new();

	/* ---------------------------------------------------------- *
	 * Load the certificate and cacert chain from file (PEM).     *
	 * ---------------------------------------------------------- */
	ret = BIO_read_filename(certbio, cert_filestr);
	if (! (cert = PEM_read_bio_X509(certbio, NULL, 0, NULL))) {
		BIO_printf(outbio, "Error loading cert into memory\n");
		exit(-1);
	}

	ret = X509_STORE_load_locations(store, ca_bundlestr, NULL);
	if (ret != 1)
		BIO_printf(outbio, "Error loading CA cert or chain file\n");

	/* ---------------------------------------------------------- *
	 * Initialize the ctx structure for a verification operation: *
	 * Set the trusted cert store, the unvalidated cert, and any  *
	 * potential certs that could be needed (here we set it NULL) *
	 * ---------------------------------------------------------- */
	X509_STORE_CTX_init(vrfy_ctx, store, cert, NULL);

	/* ---------------------------------------------------------- *
	 * Check the complete cert chain can be build and validated.  *
	 * Returns 1 on success, 0 on verification failures, and -1   *
	 * for trouble with the ctx object (i.e. missing certificate) *
	 * ---------------------------------------------------------- */
	ret = X509_verify_cert(vrfy_ctx);
	BIO_printf(outbio, "Verification return code: %d\n", ret);

	if(ret == 0 || ret == 1)
		BIO_printf(outbio, "Verification result text: %s\n",
				   X509_verify_cert_error_string(X509_STORE_CTX_get_error(vrfy_ctx)));

	/* ---------------------------------------------------------- *
	 * The error handling below shows how to get failure details  *
	 * from the offending certificate.                            *
	 * ---------------------------------------------------------- */
	if(ret == 0) {
		/*  get the offending certificate causing the failure */
		error_cert  = X509_STORE_CTX_get_current_cert(vrfy_ctx);
		certsubject = X509_NAME_new();
		certsubject = X509_get_subject_name(error_cert);
		BIO_printf(outbio, "Verification failed cert:\n");
		X509_NAME_print_ex(outbio, certsubject, 0, XN_FLAG_MULTILINE);
		BIO_printf(outbio, "\n");
	}

	/* ---------------------------------------------------------- *
	 * Free up all structures                                     *
	 * ---------------------------------------------------------- */
	X509_STORE_CTX_free(vrfy_ctx);
	X509_STORE_free(store);
	X509_free(cert);
	BIO_free_all(certbio);
	BIO_free_all(outbio);
	exit(0);
}
