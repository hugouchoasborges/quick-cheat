# 0.2.0-alpha.8 — Leitura completa do workflow

## Aceitação e publicação — 2026-10-18
Hugo confirmou: “Testei e validei tudo” e autorizou publicar uma nova versão no GitHub com documentação e capturas novas de dados de demonstração. A aceitação manual abaixo foi concluída pelo usuário. Preparação do release inclui build Release nova, capturas DEMO isoladas e pacote Windows x64 com licenças/checksums. Nenhuma nova suíte automatizada foi adicionada ou executada. Limitações gerais de compatibilidade e matriz de DPI/monitores continuam documentadas.

## Alterações
Selecionar o workflow pai na árvore mostra título e todas as etapas no leitor, em ordem, com cabeçalhos Step N of M e divisores horizontais entre elas. Selecionar uma etapa filha continua mostrando só aquela etapa. Markdown, links, imagens locais, rolagem e temas são preservados.

Cada etapa é processada separadamente pelo MD4C e inserida no mesmo Rich Edit. Código/listas ou Markdown incompleto numa etapa não engolem as seguintes nem seus divisores. O cache armazena as seções e offsets de links continuam absolutos. Capacidade do leitor ampliada de 16384 para 1048576 caracteres, acomodando os 128×4096 caracteres do armazenamento mais formatação, sem novo schema.

Mantido orçamento de oito previews de imagens por leitor para limitar memória, incluindo a visão completa. Imagens adicionais recebem indicação para selecionar a etapa correspondente; o conteúdo textual de todas as etapas é mostrado. Conteúdo é liberado ao ocultar/trocar aba, como antes. Dados pessoais e schema v3 não alterados.

## Evidências
- Build Release x64 com scripts/build.ps1 -SkipTests concluída. Nenhum teste adicionado ou executado.
- Revisão independente por inspeção: nenhum defeito concreto encontrado em cache, limites entre parsers, links e orçamento de imagens. Nenhum teste/build executado pelo revisor.
- Capturas DEMO: corrigidos divisores que podiam quebrar linha, usando separador compacto. Experimento com borda de parágrafo não foi mantido por não aparecer no controle. Capturador reafirma o alvo externo a cada tema e sincroniza repintura: interferência de outro app no foreground invalidou uma captura anterior.
- Confirmação final Dark/Light mostra o workflow pai com três etapas e divisores sem quebra. Captura da seleção filha mantém Markdown e imagem locais. Nenhuma alegação de validação completa de monitores/DPI ou workflows no limite máximo.
- git diff --check sem erros. Executável padrão atualizado; aceitação manual confirmada por Hugo.

## Roteiro de aceitação manual confirmado pelo usuário
1. No ChatGPT, abrir Workflows e clicar no nome de cada fluxo: todas as etapas devem aparecer, numeradas e separadas. A árvore pode ficar recolhida sem impedir a leitura completa.
2. Selecionar filhos e voltar ao pai: leitura individual e completa devem alternar corretamente, com texto, Markdown, links e imagens.
3. Conferir rolagem em fluxo longo, redimensionamento/Expand/Restore e temas. Nenhuma etapa posterior deve desaparecer por um bloco de código incompleto anterior.
4. Fluxos com muitas imagens: conferir placeholder após oito previews e acesso às imagens pela seleção individual.
5. Fechar/reabrir popup e trocar aplicativos/abas: leitor não deve manter conteúdo de outro contexto. Cadastro e dados salvos permanecem intactos.

## Entrega
feature/0.2.0-workflow-overview → develop → release/0.2.0-alpha.8. Publicação autorizada como v0.2.0-alpha.8; main recebe esta versão após o release ser publicado.

## Capturas da publicação
Novas capturas da build Release com perfil fictício Visual Studio Code (demo), pasta isolada e diagramas originais. [Settings Light](images/settings-general-light.png) e [Workflow completo Light](images/workflow-overview-light.png) complementam as imagens novas do README. Dados e anexos pessoais não incluídos.
