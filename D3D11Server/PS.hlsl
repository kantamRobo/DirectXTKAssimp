
struct PSOut
{
    float4 color0 : SV_Target0; // rtvWin
    float4 color1 : SV_Target1; // rtvOff
};
PSOut PSMain() : SV_Target
{
    PSOut o;
    o.color0 = float4(1, 0, 0, 1);
    o.color1 = float4(1, 0, 0, 1);
    return o;
}


/*

2. �}���`�����_�[�^�[�Q�b�g�Ή��̃s�N�Z���V�F�[�_�ɂ���
�����P��̃h���[�R�[���ŗ����ɓ����ɏo�͂������Ȃ�APS ������ȕ��ɏ��������܂��F

hlsl
�R�s�[����
�ҏW����
struct PSOut {
    float4 color0 : SV_Target0;  // rtvWin
    float4 color1 : SV_Target1;  // rtvOff
};

PSOut PSMain(...) {
    PSOut o;
    o.color0 = float4(1,0,0,1);
    o.color1 = float4(1,0,0,1);
    return o;
}
���̏ꍇ�͏]���ǂ���

cpp
�R�s�[����
�ҏW����
ctx->OMSetRenderTargets(2, rtvs, nullptr);
ctx->Draw(count, 0);
swap->Present(1, 0);
ctx->CopyResource(stageTex, offTex);
�Ƃ���΁A��x�ŗ����ɎO�p�`���`����܂��B

�ȏ�̂����ꂩ�ŁA�I�t�X�N���[���p�e�N�X�`���ɂ������ƎO�p�`���`�����܂��悤�ɂȂ�A�z�X�g���̕\�����Ԃ��O�p�`�ɂȂ�͂��ł��B
*/