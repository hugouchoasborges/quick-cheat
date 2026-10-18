# 0.1.0-alpha.4 — execução por clique duplo

## Decisão e implementação
Pedido de Hugo: clique duplo numa linha da popup também executa o atalho. Clique simples continua selecionando. Execute e Enter continuam disponíveis.
Tratamento nativo de WM_LBUTTONDBLCLK com verificação da linha e uso do mesmo caminho App::execute, mantendo validações do aplicativo de destino. Formato de dados permanece v2.

## Validação e aceitação
Build Release x64 compilada com sucesso via scripts/build.ps1 -SkipTests. Testes automatizados e interação manual não foram executados nesta alteração.
Aceitação pendente: clique simples não executa; clique duplo executa uma vez no destino correto; duplo clique na área vazia não executa; Execute e Enter continuam funcionando; troca de aplicativo não envia ao destino errado.
Gitflow: feature/0.1.0-double-click, develop e release/0.1.0-alpha.4. main permanece reservada ao release publicado.

## Aceitação concluída — 2026-10-18

Hugo confirmou: Testei tudo e está funcionando. A alpha.4 está aceita pelo usuário após testes manuais. Este registro substitui o estado anterior de aceitação pendente. Release público ainda não publicado.

## Preparação do primeiro release público
README reescrito em inglês com descrição simples, download, uso, armazenamento local e limites. Capturas reais da popup Dark/Light e cadastro Dark geradas pelo qc_preview com perfil fictício Visual Studio Code (demo), em diretório separado dos dados pessoais. Imagens em docs/images; dados DEMO não são distribuídos como perfis do usuário.
Build Release x64 recompilada com sucesso. Capturas inspecionadas visualmente; nenhuma suíte de testes adicional executada nesta preparação. Aceitação manual da alpha.4 permanece a confirmação do Hugo registrada acima.
