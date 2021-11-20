#include <stdio.h>
#include <tss2/tss2_fapi.h>

int main()
{
	TSS2_RC ret = 0;
	char *info = NULL;
	FAPI_CONTEXT *ctx = NULL;

	ret = Fapi_Initialize(&ctx, NULL);
	if (ret != TSS2_RC_SUCCESS) {
		fprintf(stderr, "Failed to init: %d\n", ret);
		return ret;
	}

	ret = Fapi_GetInfo(ctx, &info);
	if (ret != TSS2_RC_SUCCESS) {
		fprintf(stderr, "Failed to get info: 0x%x\n", ret);
		goto clean;
	}

	printf("TPM Info: \n%s\n", info);
	Fapi_Free(info);

	ret = Fapi_Provision(ctx, NULL, NULL, NULL);
	if (ret != TSS2_RC_SUCCESS) {
		fprintf(stderr, "Failed to provision: 0x%x\n", ret);
		goto clean;
	}
	
	ret = Fapi_List(ctx, "/", &info);
	if (ret != TSS2_RC_SUCCESS) {
		fprintf(stderr, "Failed to list: 0x%x\n", ret);
		goto clean;
	}

	printf("TPM List:\n%s\n", info);
	Fapi_Free(info);
clean:
	Fapi_Finalize(&ctx);
	return 0;
}
