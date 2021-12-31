function [HOTA, DETA, ASSA, HOTAa, DetAa, AssAa] = hota(tkBoxes, tkIDs, gtBoxes, gtIDs)


%=======================================================================================
% author: Song Wang and Zhu Wang
%
%
% 2021-11-29
%=======================================================================================


HOTA = 0;
DETA = 0;
ASSA = 0;

DetAa = zeros(19, 1);
AssAa = zeros(19, 1);
HOTAa = zeros(19, 1);

mat = cell(numel(tkIDs), 1);
for i = 1 : numel(tkIDs)
    for j = 1 : size(tkBoxes{i}, 1)
        for k = 1 : size(gtBoxes{i}, 1)
            mat{i}(j, k) = bboxOverlapRatio(double(tkBoxes{i}(j, : )), gtBoxes{i}(k, : ));
        end
    end
end

for alpha = 0.05 : 0.05 : 0.95
    
    TP = 0;
    FP = 0;
    FN = 0;
    
    ass = 0;
    n = 0;
    
    for i = 1 : numel(tkIDs)
        if isempty(mat{i})
            continue;
        end
        
        itp = assignmunkres(1 - mat{i}, 1);
        itp = itp(mat{i}(sub2ind(size(mat{i}), itp( : , 1), itp( : , 2))) > alpha, : );
        
        tp = size(itp, 1);
        TP = TP + tp;
        FP = FP + size(mat{i}, 1) - tp;
        FN = FN + size(mat{i}, 2) - tp;
        
        if tp == 0
            continue;
        else
            n = n + 1;
        end
        
        TPA = 0;
        FPA = 0;
        FNA = 0;
        
        for j = 1 : numel(tkIDs)
            if isempty(tkIDs{j}) || isempty(gtIDs{j})
                continue;
            end
            
            jtp = assignmunkres(1 - mat{j}, 1);
            jtp = jtp(mat{j}(sub2ind(size(mat{j}), jtp( : , 1), jtp( : , 2))) > alpha, : );
            
            [is, ids] = ismember(tkIDs{j}, tkIDs{i}(itp( : , 1)));
            gtids = gtIDs{i}(itp( : , 2));
            ids(is) = gtids(ids(is));
            
            tpa = sum(ids(jtp( : , 1)) == gtIDs{j}(jtp( : , 2)));
            fpa = sum(is) - tpa;
            fna = sum(ismember(gtIDs{j}, gtIDs{i}(itp( : , 2)))) - tpa;
            
            TPA = TPA + tpa;
            FPA = FPA + fpa;
            FNA = FNA + fna;
        end
        
        ass = ass + TPA / (TPA + FPA + FNA);
    end
    
    if n > 0
        det = TP / (TP + FP + FN);
        ass = ass / n;
    else
        det = 0;
        ass = 0;
    end
    
    DETA = DETA + det;
    ASSA = ASSA + ass;
    HOTA = HOTA + sqrt(det * ass);
    
    idx = round(alpha / 0.05);
    DetAa(idx) = det;
    AssAa(idx) = ass;
    HOTAa(idx) = sqrt(det * ass);
end

DETA = DETA / 19;
ASSA = ASSA / 19;
HOTA = HOTA / 19;






